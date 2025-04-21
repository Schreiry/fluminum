// --- Combined Single File: Strassen Matrix Multiplication (Enhanced Version - Corrected) ---

// Define NOMINMAX *before* including windows.h to prevent macro conflicts
#define NOMINMAX

// --- Includes ---
#include <windows.h> // For Windows API (Memory status, Process info, SetConsoleOutputCP)
#include <psapi.h>   // For GetProcessMemoryInfo
#include <iostream>  // For input/output (cout, cin, cerr)
#include <vector>    // For std::vector (used for matrix data)
#include <string>    // For std::string
#include <thread>    // For std::thread::hardware_concurrency
#include <stdexcept> // For exception classes (std::runtime_error, etc.)
#include <random>    // For random number generation
#include <iomanip>   // For std::setprecision, std::fixed, std::setw
#include <cmath>     // For std::log2, std::ceil, std::pow
#include <future>    // For std::async, std::future
#include <chrono>    // For timing (high_resolution_clock)
#include <atomic>    // For std::atomic (thread-safe counter)
#include <algorithm> // For std::max, std::min, std::swap
#include <limits>    // For std::numeric_limits (used in input clearing, number range)
#include <functional>// For std::ref (used with std::async for references)
#include <fstream>   // For file input/output (std::ifstream, std::ofstream)
#include <sstream>   // For string stream processing (std::stringstream)

#include <io.h>
#include <fcntl.h>



// Using directives for common elements, as in the original code structure
using std::string;
using std::cout;
using std::cin;
using std::endl;
using std::cerr;

// Link with Psapi.lib (Specific to MSVC compiler, use -lpsapi for g++)
#pragma comment(lib, "Psapi.lib")

// --- Console Formatting ---
const string RED = "\033[1;31m";
const string GREEN = "\033[1;32m";
const string YELLOW = "\033[1;33m";
const string BLUE = "\033[1;34m";
const string PURPLE = "\033[1;35m";
const string CYAN = "\033[1;36m";
const string RESET = "\033[0m";
const string DARK_GRAY = "\033[0;30m"; // Often appears as gray background or dark text
const string LIGHT_GRAY = "\033[0;37m"; // Standard light gray text

// Simple box drawing characters (adjust based on terminal support)
// These are Unicode characters and require UTF-8 console encoding and a supporting font.
const string BOX_HLINE = "\u2500"; // ─
const string BOX_VLINE = "\u2502"; // │
const string BOX_TLCORNER = "\u250C"; // ┌
const string BOX_TRCORNER = "\u2510"; // ┐
const string BOX_BLCORNER = "\u2514"; // └
const string BOX_BRCORNER = "\u2518"; // ┘
const string BOX_RTEE = "\u2524"; // ├
const string BOX_LTEE = "\u251C"; // ┤
const string BOX_BTEE = "\u2534"; // ┴
const string BOX_TTEE = "\u252C"; // ┬
const string BOX_CROSS = "\u253C"; // ┼


void print_hline(int width) {
    for (int i = 0; i < width; ++i) cout << BOX_HLINE;
}

void print_header_box(const string& title, int width = 80) {
    int title_len = title.length();
    int padding_total = width - title_len - 4;
    int padding_left = padding_total / 2;
    int padding_right = padding_total - padding_left;

    cout << BOX_TLCORNER; print_hline(width - 2); cout << BOX_TRCORNER << endl;
    cout << BOX_VLINE << BLUE << std::string(padding_left, ' ') << title << std::string(padding_right, ' ') << RESET << BOX_VLINE << endl;
    cout << BOX_LTEE; print_hline(width - 2); cout << BOX_RTEE << endl;
}

void print_footer_box(int width = 80) {
    cout << BOX_BLCORNER; print_hline(width - 2); cout << BOX_BRCORNER << endl;
}

// --- SystemInfo Implementation ---
struct SystemMemoryInfo {
    unsigned long long totalPhysicalMB;
    unsigned long long availablePhysicalMB;
};

struct ProcessMemoryInfo {
    size_t peakWorkingSetMB;
};

SystemMemoryInfo getSystemMemoryInfo() {
    MEMORYSTATUSEX statex;
    statex.dwLength = sizeof(statex);
    if (!GlobalMemoryStatusEx(&statex)) {
        DWORD error = GetLastError();
        cerr << RED << "Warning: Failed to get system memory status (Error " << error << "). Reporting 0 MB." << RESET << endl;
        return { 0, 0 };
    }
    return { statex.ullTotalPhys / (1024 * 1024), statex.ullAvailPhys / (1024 * 1024) };
}

unsigned int getCpuCoreCount() {
    unsigned int cores = std::thread::hardware_concurrency();
    return (cores == 0) ? 1 : cores;
}

ProcessMemoryInfo getProcessMemoryUsage() {
    PROCESS_MEMORY_COUNTERS pmc;
    ZeroMemory(&pmc, sizeof(pmc));
    pmc.cb = sizeof(pmc);
    if (GetProcessMemoryInfo(GetCurrentProcess(), &pmc, sizeof(pmc))) {
        return { pmc.PeakWorkingSetSize / (1024 * 1024) };
    }
    else {
        DWORD error = GetLastError();
        cerr << RED << "Warning: Failed to get process memory info (Error " << error << "). Reporting 0 MB." << RESET << endl;
        return { 0 };
    }
}

// Improved Memory Estimation Explanation
// This is an *approximate* upper bound heuristic based on allocating multiple temporary
// matrices of size N x N at a single level of recursion (e.g., S_i and P_i matrices).
// The actual peak memory usage during parallel recursion is complex and depends on
// recursion depth, thread scheduling, and how matrix operations are implemented.
// The reported Peak Working Set Size after computation is the more accurate value.
unsigned long long estimateStrassenMemoryMB(int n) {
    if (n <= 0) return 0;
    unsigned long long elementSize = sizeof(double);
    unsigned long long numElements = static_cast<unsigned long long>(n) * static_cast<unsigned long long>(n);
    // Estimate based on needing space for original matrices + intermediate S and P matrices.
    // A simplified view suggests needing space for A, B, C + 10 S matrices + 7 P matrices
    // at a given level, roughly 20N^2 elements. Let's use a more conservative ~12N^2 as
    // a heuristic for temporary allocations during recursion levels.
    unsigned long long estimatedTotalElements = numElements * 12;
    unsigned long long estimatedTotalBytes = estimatedTotalElements * elementSize;
    return estimatedTotalBytes / (1024 * 1024);
}


// --- Matrix Class Implementation ---
class Matrix {
public:
    // Constructors
    Matrix() : rows_(0), cols_(0) {}
    Matrix(int rows, int cols) : rows_(rows), cols_(cols) {
        if (rows < 0 || cols < 0) throw std::invalid_argument("Matrix dimensions cannot be negative.");
        unsigned long long numElements = static_cast<unsigned long long>(rows) * static_cast<unsigned long long>(cols);
        if (numElements > data_.max_size()) throw std::bad_alloc(); // Check if total elements exceed vector max size
        data_.resize(static_cast<size_t>(numElements), 0.0);
    }
    Matrix(int rows, int cols, double initialValue) : rows_(rows), cols_(cols) {
        if (rows < 0 || cols < 0) throw std::invalid_argument("Matrix dimensions cannot be negative.");
        unsigned long long numElements = static_cast<unsigned long long>(rows) * static_cast<unsigned long long>(cols);
        if (numElements > data_.max_size()) throw std::bad_alloc(); // Check if total elements exceed vector max size
        data_.resize(static_cast<size_t>(numElements), initialValue);
    }
    // Constructor from 2D vector (used internally by file/console read)
    Matrix(const std::vector<std::vector<double>>& data) {
        if (data.empty() || data[0].empty()) {
            rows_ = 0; cols_ = 0;
            data_.clear(); // Ensure data_ is empty for 0x0 matrix
            return;
        }
        rows_ = static_cast<int>(data.size());
        cols_ = static_cast<int>(data[0].size());
        // Validate consistent row lengths
        for (size_t i = 1; i < data.size(); ++i) {
            if (data[i].size() != static_cast<size_t>(cols_)) {
                rows_ = 0; cols_ = 0; data_.clear();
                throw std::invalid_argument("Inconsistent row lengths in input data.");
            }
        }

        unsigned long long numElements = static_cast<unsigned long long>(rows_) * static_cast<unsigned long long>(cols_);
        if (numElements > data_.max_size()) throw std::bad_alloc();
        data_.resize(static_cast<size_t>(numElements));

        for (int i = 0; i < rows_; ++i) {
            for (int j = 0; j < cols_; ++j) {
                (*this)(i, j) = data[i][j];
            }
        }
    }


    // Accessors
    int rows() const { return rows_; }
    int cols() const { return cols_; }
    double& operator()(int r, int c) {
        if (r < 0 || r >= rows_ || c < 0 || c >= cols_) throw std::out_of_range("Matrix index out of range.");
        return data_[static_cast<size_t>(r) * cols_ + c];
    }
    double operator()(int r, int c) const {
        if (r < 0 || r >= rows_ || c < 0 || c >= cols_) throw std::out_of_range("Matrix index out of range.");
        return data_[static_cast<size_t>(r) * cols_ + c];
    }

    // Matrix operations
    Matrix operator+(const Matrix& other) const {
        if (rows_ != other.rows_ || cols_ != other.cols_) throw std::invalid_argument("Matrix dimensions must match for addition.");
        Matrix result(rows_, cols_);
        for (size_t i = 0; i < data_.size(); ++i) result.data_[i] = data_[i] + other.data_[i];
        return result;
    }
    Matrix operator-(const Matrix& other) const {
        if (rows_ != other.rows_ || cols_ != other.cols_) throw std::invalid_argument("Matrix dimensions must match for subtraction.");
        Matrix result(rows_, cols_);
        for (size_t i = 0; i < data_.size(); ++i) result.data_[i] = data_[i] - other.data_[i];
        return result;
    }

    // Naive multiplication (can be a target for SIMD, but left as standard for portability)
    Matrix multiply_naive(const Matrix& other) const {
        if (cols_ != other.rows_) throw std::invalid_argument("Matrix dimensions incompatible for multiplication (A.cols != B.rows).");
        Matrix result(rows_, other.cols_);
        for (int i = 0; i < rows_; ++i) {
            for (int j = 0; j < other.cols_; ++j) {
                double sum = 0.0;
                for (int k = 0; k < cols_; ++k) sum += (*this)(i, k) * other(k, j);
                result(i, j) = sum;
            }
        }
        return result;
    }

    // Static functions for creation
    // Generate random doubles within the approximate range of signed 32-bit integers
    static Matrix generateRandom(int rows, int cols) {
        if (rows <= 0 || cols <= 0) throw std::invalid_argument("Matrix dimensions must be positive for random generation.");

        // Define the range using limits of signed 32-bit int, cast to double
        const double minVal = static_cast<double>(std::numeric_limits<int>::min()); // Approx -2.147e9
        const double maxVal = static_cast<double>(std::numeric_limits<int>::max()); // Approx +2.147e9

        Matrix result(rows, cols);
        std::random_device rd;
        std::mt19937 gen(rd());
        // Generate doubles within the specified wide range
        std::uniform_real_distribution<double> distrib(minVal, maxVal);

        for (size_t i = 0; i < result.data_.size(); ++i) {
            result.data_[i] = distrib(gen);
        }
        return result;
    }

    static Matrix identity(int n) {
        if (n <= 0) throw std::invalid_argument("Identity matrix dimension must be positive.");
        Matrix result(n, n);
        for (int i = 0; i < n; ++i) result(i, i) = 1.0;
        return result;
    }

    // User Input functions
    static Matrix readFromConsole(int rows, int cols) {
        if (rows <= 0 || cols <= 0) throw std::invalid_argument("Matrix dimensions must be positive for console input.");
        Matrix result(rows, cols);
        cout << CYAN << "Enter elements for the " << rows << "x" << cols << " matrix:" << RESET << endl;
        cout << DARK_GRAY << "(Enter space or comma separated numbers for each row, press Enter after each row)" << RESET << endl;

        for (int i = 0; i < rows; ++i) {
            cout << "Row " << (i + 1) << "/" << rows << ": ";
            string line;
            // Use std::ws to consume leading whitespace before reading the line
            if (!std::getline(cin >> std::ws, line)) {
                throw std::runtime_error("Failed to read line from console.");
            }

            std::stringstream ss(line);
            double val;
            int j = 0;
            char comma; // To consume comma if present

            while (j < cols && ss >> val) {
                result(i, j) = val;
                j++;
                // Try to consume a comma and potential whitespace after it
                ss >> std::ws; // consume whitespace
                if (ss.peek() == ',') {
                    ss >> comma; // consume comma
                }
                ss >> std::ws; // consume whitespace after comma
            }


            if (j != cols || !ss.eof()) { // Check if correct number of elements read and no extra data
                cerr << RED << "Error reading row " << (i + 1) << ". Expected " << cols << " numbers, read " << j << ". Please try again." << RESET << endl;
                // Clear the input stream and repeat the loop iteration for the current row
                cin.clear(); // Clear any error flags on cin
                // No need to ignore the line as getline already consumed it.
                for (int k = 0; k < cols; ++k) result(i, k) = 0.0; // Clear values for this row
                i--; // Repeat current row input
            }
        }
        cout << GREEN << "Matrix input complete." << RESET << endl;
        return result;
    }

    static Matrix readFromFile(const std::string& filename) {
        std::ifstream infile(filename);
        if (!infile.is_open()) {
            throw std::runtime_error("Could not open file: " + filename);
        }

        std::vector<std::vector<double>> temp_data;
        string line;
        int expected_cols = -1;
        int row_count = 0;

        cout << CYAN << "Reading matrix from file: " << filename << RESET << endl;

        while (std::getline(infile, line)) {
            std::vector<double> row;
            std::stringstream ss(line);
            double val;
            char comma; // To consume comma if present

            while (ss >> val) {
                row.push_back(val);
                // Try to consume a comma and potential whitespace after it
                ss >> std::ws; // consume whitespace
                if (ss.peek() == ',') {
                    ss >> comma; // consume comma
                }
                ss >> std::ws; // consume whitespace after comma
            }


            if (row.empty()) continue; // Skip empty lines

            if (expected_cols == -1) {
                expected_cols = static_cast<int>(row.size());
            }
            else if (static_cast<int>(row.size()) != expected_cols) {
                infile.close();
                throw std::invalid_argument("Inconsistent number of columns in file: " + filename + " at row " + std::to_string(row_count + 1));
            }
            temp_data.push_back(row);
            row_count++;
        }

        infile.close();

        if (temp_data.empty()) {
            cout << YELLOW << "Warning: File was empty or contained no valid data. Creating 0x0 matrix." << RESET << endl;
            return Matrix(0, 0);
        }

        cout << GREEN << "Successfully read " << row_count << " rows and " << expected_cols << " columns from file." << RESET << endl;
        return Matrix(temp_data);
    }

    // Save result function
    void saveToFile(const std::string& filename, char separator = ' ') const {
        std::ofstream outfile(filename);
        if (!outfile.is_open()) {
            throw std::runtime_error("Could not open file for writing: " + filename);
        }

        cout << CYAN << "Saving matrix to file: " << filename << RESET << endl;
        outfile << std::scientific << std::setprecision(10); // Use scientific notation for potential large values

        for (int i = 0; i < rows_; ++i) {
            for (int j = 0; j < cols_; ++j) {
                outfile << (*this)(i, j) << (j == cols_ - 1 ? "" : std::string(1, separator));
            }
            outfile << endl;
        }

        outfile.close();
        if (outfile.fail() && !outfile.bad()) {
            // fail() is true if a formatting or extraction error occurs
            // bad() is true if a severe error occurs (e.g., I/O error)
            // If fail() is true but not bad(), it might be recoverable, but here we treat it as an error.
            cerr << RED << "Error writing to file: " << filename << RESET << endl;
            // Depending on needs, you might check outfile.bad() for critical errors
            // or outfile.good() for perfect status. fail() covers most writing issues.
        }
        else if (outfile.bad()) {
            cerr << RED << "Severe I/O error writing to file: " << filename << RESET << endl;
        }
        else {
            cout << GREEN << "Matrix successfully saved to " << filename << RESET << endl;
        }
    }


    // Strassen specific helpers
    void split(Matrix& A11, Matrix& A12, Matrix& A21, Matrix& A22) const {
        if (rows_ != cols_ || rows_ % 2 != 0) throw std::logic_error("Internal Error: Matrix must be square and have even dimensions for splitting.");
        int n2 = rows_ / 2;
        A11 = Matrix(n2, n2); A12 = Matrix(n2, n2);
        A21 = Matrix(n2, n2); A22 = Matrix(n2, n2);
        for (int i = 0; i < n2; ++i) {
            for (int j = 0; j < n2; ++j) {
                A11(i, j) = (*this)(i, j); A12(i, j) = (*this)(i, j + n2);
                A21(i, j) = (*this)(i + n2, j); A22(i, j) = (*this)(i + n2, j + n2);
            }
        }
    }
    static Matrix combine(const Matrix& C11, const Matrix& C12, const Matrix& C21, const Matrix& C22) {
        int n2 = C11.rows();
        if (C11.cols() != n2 || C12.rows() != n2 || C12.cols() != n2 || C21.rows() != n2 || C21.cols() != n2 || C22.rows() != n2 || C22.cols() != n2)
            throw std::invalid_argument("Quadrant dimensions must match for combining.");
        int n = n2 * 2;
        Matrix C(n, n);
        for (int i = 0; i < n2; ++i) {
            for (int j = 0; j < n2; ++j) {
                C(i, j) = C11(i, j); C(i, j + n2) = C12(i, j);
                C(i + n2, j) = C21(i, j); C(i + n2, j + n2) = C22(i, j);
            }
        }
        return C;
    }

    // Pad matrix to a target square size with zeros
    static Matrix pad(const Matrix& A, int targetSize) {
        if (targetSize < A.rows() || targetSize < A.cols()) throw std::invalid_argument("Target size for padding must be >= original dimensions.");
        if (targetSize <= 0) return Matrix(0, 0); // Handle padding to 0 or negative size

        Matrix padded(targetSize, targetSize, 0.0);
        for (int i = 0; i < A.rows(); ++i) {
            for (int j = 0; j < A.cols(); ++j) {
                // Check bounds just in case (should be covered by targetSize check above)
                if (i < targetSize && j < targetSize) {
                    padded(i, j) = A(i, j);
                }
            }
        }
        return padded;
    }

    // Unpad matrix back to original dimensions
    static Matrix unpad(const Matrix& A, int originalRows, int originalCols) {
        if (originalRows <= 0 || originalCols <= 0) {
            // If original was 0x0, return 0x0.
            if (originalRows == 0 && originalCols == 0) return Matrix(0, 0);
            throw std::invalid_argument("Original dimensions for unpadding must be positive.");
        }
        if (originalRows > A.rows() || originalCols > A.cols()) throw std::invalid_argument("Original dimensions cannot exceed padded dimensions for unpadding.");

        Matrix unpadded(originalRows, originalCols);
        for (int i = 0; i < originalRows; ++i) {
            for (int j = 0; j < originalCols; ++j) {
                unpadded(i, j) = A(i, j);
            }
        }
        return unpadded;
    }

    // Utility
    // Use scientific notation for printing large numbers from random generation
    void print(std::ostream& os = std::cout, int precision = 3, int max_print_dim = 10) const {
        std::ios_base::fmtflags original_flags = os.flags();
        std::streamsize original_precision = os.precision();

        // Decide whether to use scientific based on scale or just use fixed for small matrices
        // For simplicity, sticking to scientific as in the original for potentially large values.
        os << std::scientific << std::setprecision(precision);

        os << YELLOW << "Matrix (" << rows_ << "x" << cols_ << "):" << RESET << "\n";

        int print_rows = std::min(rows_, max_print_dim);
        int print_cols = std::min(cols_, max_print_dim);

        for (int i = 0; i < print_rows; ++i) {
            os << "[ ";
            for (int j = 0; j < print_cols; ++j) {
                os << (*this)(i, j) << (j == print_cols - 1 ? "" : "  ");
            }
            if (cols_ > print_cols) os << " ... ";
            os << " ]\n";
        }
        if (rows_ > print_rows) os << "  ...\n";
        os << std::endl;

        os.flags(original_flags); // Restore default float format
        os.precision(original_precision);
    }
    bool isEmpty() const { return rows_ == 0 || cols_ == 0; }
    size_t elementCount() const { return static_cast<size_t>(rows_) * static_cast<size_t>(cols_); }

private:
    int rows_;
    int cols_;
    std::vector<double> data_;
};


// --- StrassenMultiply Implementation ---
struct MultiplicationResult {
    Matrix resultMatrix;
    double durationSeconds;
    unsigned int threadsUsed;
    unsigned int coresDetected;
    ProcessMemoryInfo memoryInfo;
    int strassenThreshold; // Store the threshold used
};

int nextPowerOf2(int n) {
    if (n <= 0) return 1; // Smallest power of 2 is 1 (2^0)
    // Handle potential overflow for very large n
    // std::numeric_limits<int>::max() is approx 2e9. Shifting 1 left 30 times is approx 1e9.
    // Shifting 1 left 31 times would overflow a signed int.
    // Let's check if n is larger than half of the max int value + 1
    if (n > (std::numeric_limits<int>::max() / 2) + 1) {
        // Check against a value known not to overflow when shifted left
        // A safer upper bound check might be related to the maximum possible matrix size squared
        // that can fit in size_t for vector indexing.
        // For int, next power of 2 can go up to the largest power of 2 <= INT_MAX.
        // For N*N matrix, N up to sqrt(SIZE_MAX / sizeof(double)).
        // Let's assume int is 32-bit. Max N can be ~2^30. nextPowerOf2 will fit.
        // A value like (1 << 30) is fine. (1 << 31) overflows.
        // If n > (1 << 30), next power of 2 will be > (1 << 30).
        // Checking against a safe large value like 2^29 or 2^30 might be better than INT_MAX/2
        // but the simple check protects against immediate overflow in the loop.
    }
    int power = 1;
    while (power < n) {
        // Check for potential overflow *before* shifting
        if (power > std::numeric_limits<int>::max() / 2) {
            throw std::overflow_error("Input size is too large; next power of 2 would exceed integer limit.");
        }
        power <<= 1;
    }
    return power;
}

// --- IMPORTANT: This function is a standalone function ---
Matrix strassen_parallel_internal(const Matrix& A, const Matrix& B, int threshold, int current_depth, int max_depth);

struct MultiplicationResult multiplyStrassenParallel(const Matrix& A, const Matrix& B, int threshold, unsigned int num_threads_request = 0) {
    if (A.cols() != B.rows()) throw std::invalid_argument("Matrix dimensions incompatible (A.cols != B.rows).");
    if (A.isEmpty() || B.isEmpty()) {
        // Get current stats even for empty result
        ProcessMemoryInfo memInfo = getProcessMemoryUsage();
        unsigned int coreCount = getCpuCoreCount();
        // Result matrix dimensions should be A.rows() x B.cols() even if inputs are empty (resulting in 0x0)
        return { Matrix(A.rows(), B.cols()), 0.0, 0, coreCount, memInfo, threshold };
    }

    unsigned int hardware_cores = getCpuCoreCount();
    // Use specified threads, or hardware cores if 0 is requested.
    // Cap requested threads at hardware cores for potentially better efficiency.
    unsigned int threads_to_use = (num_threads_request == 0) ? hardware_cores : std::min(num_threads_request, hardware_cores);
    if (threads_to_use == 0) threads_to_use = 1; // Ensure at least one thread

    // Calculate recursion depth limit for parallelism.
    // Log base 2 of thread count provides a reasonable depth to create tasks.
    int max_depth = (threads_to_use <= 1) ? 0 : static_cast<int>(std::max(1.0, std::log2(static_cast<double>(threads_to_use))));
    // Ensure max_depth is at least 0.
    max_depth = std::max(0, max_depth);

    // Determine required padded size for square matrices with power-of-2 dimensions
    int original_rows_A = A.rows();
    int original_cols_B = B.cols();
    int n = std::max({ original_rows_A, A.cols(), B.rows(), original_cols_B }); // Max dimension

    // Handle cases where n=0 or 1 gracefully for nextPowerOf2
    int padded_size = nextPowerOf2(n > 0 ? n : 1); // Pad to at least 1 if dimensions were 0

    // Check if padding is necessary and possible
    if (padded_size < A.rows() || padded_size < A.cols() || padded_size < B.rows() || padded_size < B.cols()) {
        throw std::runtime_error("Internal error: Calculated padded size is smaller than original matrix dimensions.");
    }
    // Check if padded size is excessively large (potential overflow issues with matrix data size)
    unsigned long long padded_elements = static_cast<unsigned long long>(padded_size) * padded_size;
    // Check if the total number of elements in the padded matrix would exceed the maximum size_t / element size
    if (padded_elements > std::numeric_limits<size_t>::max() / sizeof(double)) {
        throw std::bad_alloc(); // Indicate that the padded matrix is too large for memory
    }


    const Matrix Apad = Matrix::pad(A, padded_size);
    const Matrix Bpad = Matrix::pad(B, padded_size);

    auto start_time = std::chrono::high_resolution_clock::now();

    // Call the internal recursive function (now standalone)
    Matrix Cpad = strassen_parallel_internal(Apad, Bpad, threshold, 0, max_depth);

    auto end_time = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> duration = end_time - start_time;

    // Unpad the result matrix to the original dimensions of A and B
    Matrix C = Matrix::unpad(Cpad, original_rows_A, original_cols_B);
    ProcessMemoryInfo memInfo = getProcessMemoryUsage();

    MultiplicationResult result;
    result.resultMatrix = std::move(C);
    result.durationSeconds = duration.count();
    result.threadsUsed = threads_to_use; // Report the intended parallelism
    result.coresDetected = hardware_cores;
    result.memoryInfo = memInfo;
    result.strassenThreshold = threshold;

    return result;
}

// --- IMPORTANT: This function is a standalone function ---
Matrix strassen_parallel_internal(const Matrix& A, const Matrix& B, int threshold, int current_depth, int max_depth)
{
    // Base case: Switch to naive multiplication
    // Check A.rows() because A, B will always be square and same size due to padding
    if (A.rows() <= threshold || A.rows() == 0) {
        if (A.isEmpty() || B.isEmpty()) return Matrix(A.rows(), B.cols()); // Should be 0x0
        return A.multiply_naive(B);
    }
    // Internal consistency check (should not happen if padding/splitting is correct)
    if (A.rows() != A.cols() || A.rows() != B.rows() || A.rows() % 2 != 0) {
        cerr << RED << "Internal Logic Error: strassen_parallel_internal called with non-square, unequal, or odd-dimensioned matrices (Size: " << A.rows() << "x" << A.cols() << " and " << B.rows() << "x" << B.cols() << ")." << RESET << endl;
        throw std::logic_error("Internal Error: strassen_parallel_internal called with invalid matrix dimensions.");
    }


    int n = A.rows();
    int n2 = n / 2;

    // Declare matrices for subproblems and intermediates
    // Using direct declaration rather than dynamic allocation where possible for minor efficiency
    // Note: These matrices can still consume significant stack or heap memory depending on compiler/size
    Matrix A11(n2, n2), A12(n2, n2), A21(n2, n2), A22(n2, n2);
    Matrix B11(n2, n2), B12(n2, n2), B21(n2, n2), B22(n2, n2);

    // Split matrices A and B into quadrants
    A.split(A11, A12, A21, A22);
    B.split(B11, B12, B21, B22);

    // Compute the 10 intermediate S matrices
    Matrix S1 = B12 - B22;
    Matrix S2 = A11 + A12;
    Matrix S3 = A21 + A22;
    Matrix S4 = B21 - B11;
    Matrix S5 = A11 + A22;
    Matrix S6 = B11 + B22;
    Matrix S7 = A12 - A22;
    Matrix S8 = B21 + B22;
    Matrix S9 = A21 - A11;
    Matrix S10 = B11 + B12;

    // Compute the 7 product matrices P_i recursively, potentially in parallel
    bool launch_async = (current_depth < max_depth);

    // Use futures for potentially parallel tasks
    std::future<Matrix> future_P1, future_P2, future_P3, future_P4, future_P5, future_P6, future_P7;

    if (launch_async) {
        // Launch tasks asynchronously using std::async. The runtime decides if a new thread is needed.
        // std::launch::async encourages a new thread, but not guaranteed.
        // Calls to the standalone strassen_parallel_internal
        future_P1 = std::async(std::launch::async, strassen_parallel_internal, S5, S6, threshold, current_depth + 1, max_depth);
        future_P2 = std::async(std::launch::async, strassen_parallel_internal, S3, B11, threshold, current_depth + 1, max_depth);
        future_P3 = std::async(std::launch::async, strassen_parallel_internal, A11, S1, threshold, current_depth + 1, max_depth);
        future_P4 = std::async(std::launch::async, strassen_parallel_internal, A22, S4, threshold, current_depth + 1, max_depth);
        future_P5 = std::async(std::launch::async, strassen_parallel_internal, S2, B22, threshold, current_depth + 1, max_depth);
        future_P6 = std::async(std::launch::async, strassen_parallel_internal, S9, S10, threshold, current_depth + 1, max_depth);
        future_P7 = std::async(std::launch::async, strassen_parallel_internal, S7, S8, threshold, current_depth + 1, max_depth);


        // Retrieve results (waits if necessary)
        Matrix P1 = future_P1.get();
        Matrix P2 = future_P2.get();
        Matrix P3 = future_P3.get();
        Matrix P4 = future_P4.get();
        Matrix P5 = future_P5.get();
        Matrix P6 = future_P6.get();
        Matrix P7 = future_P7.get();

        // The intermediate S and P matrices go out of scope here and are automatically destroyed
        // which helps manage memory during recursion.

        // Compute the 4 quadrants of the result matrix C
        Matrix C11 = P1 + P4 - P5 + P7;
        Matrix C12 = P3 + P5;
        Matrix C21 = P2 + P4;
        Matrix C22 = P1 - P2 + P3 + P6;

        // Combine the quadrants into the final result matrix
        return Matrix::combine(C11, C12, C21, C22);

    }
    else {
        // Execute sequentially when not launching async tasks (below max_depth)
        // This path is also taken below the threshold internally.
        // Recursive calls to the standalone strassen_parallel_internal
        Matrix P1 = strassen_parallel_internal(S5, S6, threshold, current_depth + 1, max_depth);
        Matrix P2 = strassen_parallel_internal(S3, B11, threshold, current_depth + 1, max_depth);
        Matrix P3 = strassen_parallel_internal(A11, S1, threshold, current_depth + 1, max_depth);
        Matrix P4 = strassen_parallel_internal(A22, S4, threshold, current_depth + 1, max_depth);
        Matrix P5 = strassen_parallel_internal(S2, B22, threshold, current_depth + 1, max_depth);
        Matrix P6 = strassen_parallel_internal(S9, S10, threshold, current_depth + 1, max_depth);
        Matrix P7 = strassen_parallel_internal(S7, S8, threshold, current_depth + 1, max_depth);


        Matrix C11 = P1 + P4 - P5 + P7;
        Matrix C12 = P3 + P5;
        Matrix C21 = P2 + P4;
        Matrix C22 = P1 - P2 + P3 + P6;

        return Matrix::combine(C11, C12, C21, C22);
    }
}


// --- Main Function ---

// Define PRINT_MATRICES to print matrices (disable for large ones)
// #define PRINT_MATRICES

void clear_input_buffer() {
    // Check if there are characters to ignore
    if (cin.rdbuf()->in_avail() > 0) {
        cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    }
}

template<typename T>
T get_valid_input(const std::string& prompt) {
    T value;
    while (true) {
        cout << prompt;
        cin >> value;
        if (cin.good()) {
            clear_input_buffer(); // Clear the rest of the line
            return value;
        }
        else {
            cerr << RED << "Invalid input. Please try again." << RESET << endl;
            cin.clear(); // Clear error flags
            clear_input_buffer(); // Clear the invalid input from the buffer
        }
    }
}

template<> // Specialization for string to allow spaces in filenames etc.
string get_valid_input<string>(const std::string& prompt) {
    string value;
    while (true) {
        cout << prompt;
        // Use getline after potentially clearing the buffer from previous reads
        // std::ws consumes leading whitespace
        std::getline(cin >> std::ws, value);
        if (!value.empty()) { // Basic validation: not empty
            return value;
        }
        else {
            cerr << RED << "Input cannot be empty. Please try again." << RESET << endl;
            // No need to clear cin state or buffer here, as getline handles it
        }
    }
}


int main() {
    // --- FIX: Set console output code page to UTF-8 ---
    // This is necessary to correctly display Unicode characters like box-drawing symbols.
    // Requires a console font that supports Unicode (e.g., Consolas, Cascadia Mono).
    // On Windows 10+, SetConsoleOutputCP and ENABLE_VIRTUAL_TERMINAL_PROCESSING work well with Windows Terminal.
    SetConsoleOutputCP(CP_UTF8);

    // Enable virtual terminal processing for ANSI escape codes on Windows 10+
    // Without this, colors and other ANSI sequences might not work on standard cmd.exe
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hOut != INVALID_HANDLE_VALUE) {
        DWORD dwMode = 0;
        if (GetConsoleMode(hOut, &dwMode)) {
            dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
            SetConsoleMode(hOut, dwMode);
        }
    }

    try {
        print_header_box("Strassen Matrix Multiplication");
        cout << BOX_VLINE << "                                                                " << BOX_VLINE << endl;
        cout << BOX_VLINE << "  An efficient parallel matrix multiplication algorithm.        " << BOX_VLINE << endl;
        cout << BOX_VLINE << "                                                                " << BOX_VLINE << endl;
        print_footer_box();
        cout << endl;


        // --- 1. System Information ---
        print_header_box("System Information");
        SystemMemoryInfo sysMemInfo = getSystemMemoryInfo();
        unsigned int coreCount = getCpuCoreCount();
        cout << BOX_VLINE << " Total Physical RAM     : " << PURPLE << std::setw(8) << sysMemInfo.totalPhysicalMB << " MB" << RESET << "                         " << BOX_VLINE << endl;
        cout << BOX_VLINE << " Available Physical RAM : " << GREEN << std::setw(8) << sysMemInfo.availablePhysicalMB << " MB" << RESET << "                         " << BOX_VLINE << endl;
        cout << BOX_VLINE << " Logical CPU Cores      : " << BLUE << std::setw(8) << coreCount << RESET << "                                 " << BOX_VLINE << endl;
        print_footer_box();
        cout << endl;

        // --- 2. Get Matrix Dimensions ---
        print_header_box("Matrix Dimensions Input");
        int rowsA = 0, colsA = 0, rowsB = 0, colsB = 0;

        // Use get_valid_input for cleaner integer input with validation
        cout << BOX_VLINE << " Enter dimensions for Matrix [ A ]:                " << BOX_VLINE << endl;
        cout << BOX_LTEE; print_hline(78); cout << BOX_RTEE << endl;
        rowsA = get_valid_input<int>(BOX_VLINE + string(" Rows:    ") + YELLOW); cout << RESET << BOX_VLINE << endl;
        colsA = get_valid_input<int>(BOX_VLINE + string(" Columns: ") + YELLOW); cout << RESET << BOX_VLINE << endl;

        cout << BOX_LTEE; print_hline(78); cout << BOX_RTEE << endl; // Separator

        cout << BOX_VLINE << " Enter dimensions for Matrix [ B ]:                " << BOX_VLINE << endl;
        cout << BOX_LTEE; print_hline(78); cout << BOX_RTEE << endl;
        rowsB = get_valid_input<int>(BOX_VLINE + string(" Rows:    ") + YELLOW); cout << RESET << BOX_VLINE << endl;
        colsB = get_valid_input<int>(BOX_VLINE + string(" Columns: ") + YELLOW); cout << RESET << BOX_VLINE << endl;

        print_footer_box();
        cout << endl;

        if (rowsA <= 0 || colsA <= 0 || rowsB <= 0 || colsB <= 0) {
            cerr << RED << "Error: Matrix dimensions must be positive." << RESET << endl; return 1;
        }
        if (colsA != rowsB) {
            cerr << RED << "Error: Incompatible matrix dimensions for multiplication (A.cols: " << colsA << " != B.rows: " << rowsB << ")." << RESET << std::endl; return 1;
        }

        // --- 3. Estimate Memory ---
        print_header_box("Memory Estimation");
        // Estimate based on the largest dimension relevant to padding
        int max_dim_for_padding = std::max({ rowsA, colsA, rowsB, colsB });
        // Handle 0 dimensions for nextPowerOf2
        int padded_n = nextPowerOf2(max_dim_for_padding > 0 ? max_dim_for_padding : 1);

        unsigned long long estimatedMB = estimateStrassenMemoryMB(padded_n);

        cout << BOX_VLINE << BLUE << " Estimated peak RAM needed (approx): " << std::setw(8) << estimatedMB << " MB" << RESET << "                      " << BOX_VLINE << endl;
        cout << BOX_VLINE << DARK_GRAY << " (Estimate based on padded size " << padded_n << "x" << padded_n << ")" << RESET << "                            " << BOX_VLINE << endl;
        if (sysMemInfo.availablePhysicalMB > 0 && estimatedMB > sysMemInfo.availablePhysicalMB) {
            cout << BOX_VLINE << RED << " Warning: Estimated RAM exceeds available RAM. " << RESET << BOX_VLINE << endl;
            cout << BOX_VLINE << RED << " Performance may degrade or lead to errors.  " << RESET << BOX_VLINE << endl;
        }
        else {
            cout << BOX_VLINE << GREEN << " Sufficient available RAM detected.            " << RESET << BOX_VLINE << endl;
        }
        print_footer_box();
        cout << endl;


        // --- 4. Input/Generate Matrices ---
        print_header_box("Matrix Input Method");
        cout << BOX_VLINE << " Select input method:                         " << BOX_VLINE << endl;
        cout << BOX_VLINE << " 1. Random Generation                         " << BOX_VLINE << endl;
        cout << BOX_VLINE << " 2. Manual Console Input (for small matrices)" << BOX_VLINE << endl;
        cout << BOX_VLINE << " 3. Read from File                            " << BOX_VLINE << endl;
        cout << BOX_LTEE; print_hline(78); cout << BOX_RTEE << endl;

        int input_choice = get_valid_input<int>(BOX_VLINE + string(" Enter choice (1-3): ") + YELLOW); cout << RESET << BOX_VLINE << endl;
        print_footer_box();
        cout << endl;

        Matrix A(rowsA, colsA), B(rowsB, colsB);
        auto gen_start = std::chrono::high_resolution_clock::now();

        switch (input_choice) {
        case 1:
            cout << CYAN << "Generating Matrix A (" << rowsA << "x" << colsA << ") and Matrix B (" << rowsB << "x" << colsB
                << ") with values in range [~ -2.147e9, ~ +2.147e9]..." << RESET << endl;
            A = Matrix::generateRandom(rowsA, colsA);
            B = Matrix::generateRandom(rowsB, colsB);
            break;
        case 2:
            if (rowsA > 10 || colsA > 10 || rowsB > 10 || colsB > 10) {
                cout << YELLOW << "Warning: Manual console input is recommended only for small matrices (<10x10)." << RESET << endl;
            }
            try {
                // Pass expected dimensions to console read
                A = Matrix::readFromConsole(rowsA, colsA);
                B = Matrix::readFromConsole(rowsB, colsB);
            }
            catch (const std::exception& e) {
                cerr << RED << "Error during console input: " << e.what() << RESET << endl; return 1;
            }
            break;
        case 3:
        { // Scope for filename strings
            string filenameA, filenameB;
            print_header_box("File Input");
            filenameA = get_valid_input<string>(BOX_VLINE + string(" Enter filename for Matrix [ A ]: ") + YELLOW); cout << RESET << BOX_VLINE << endl;
            filenameB = get_valid_input<string>(BOX_VLINE + string(" Enter filename for Matrix [ B ]: ") + YELLOW); cout << RESET << BOX_VLINE << endl;
            print_footer_box();
            cout << endl;
            try {
                Matrix tempA = Matrix::readFromFile(filenameA);
                Matrix tempB = Matrix::readFromFile(filenameB);
                // Validate dimensions read from file match expected dimensions
                if (tempA.rows() != rowsA || tempA.cols() != colsA) {
                    cerr << RED << "Error: Matrix A dimensions from file (" << tempA.rows() << "x" << tempA.cols() << ") do not match requested (" << rowsA << "x" << colsA << ")." << RESET << endl; return 1;
                }
                if (tempB.rows() != rowsB || tempB.cols() != colsB) {
                    cerr << RED << "Error: Matrix B dimensions from file (" << tempB.rows() << "x" << tempB.cols() << ") do not match requested (" << rowsB << "x" << colsB << ")." << RESET << endl; return 1;
                }
                A = std::move(tempA);
                B = std::move(tempB);

            }
            catch (const std::exception& e) {
                cerr << RED << "Error during file input: " << e.what() << RESET << endl; return 1;
            }
        }
        break;
        default:
            cerr << RED << "Error: Invalid input choice." << RESET << endl; return 1;
        }

        auto gen_end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> gen_duration = gen_end - gen_start;
        cout << "Matrix input/generation took: " << GREEN << gen_duration.count() << " seconds." << RESET << endl << endl;

        // --- Optional: Print Generated/Input Matrices (if enabled) ---
#ifdef PRINT_MATRICES
        print_header_box("Input Matrices Preview (First 10x10)");
        A.print(cout, 3, 10);
        B.print(cout, 3, 10);
        print_footer_box();
        cout << endl;
#else
        cout << DARK_GRAY << "(Matrix preview printing is disabled. Define PRINT_MATRICES to enable.)" << RESET << endl << endl;
#endif

        // --- 5. Get Strassen Threshold and Thread Count ---
        print_header_box("Multiplication Settings");
        int strassen_threshold = get_valid_input<int>(BOX_VLINE + string(" Enter Strassen threshold (e.g., 64 or 128): ") + YELLOW); cout << RESET << BOX_VLINE << endl;
        if (strassen_threshold <= 0) {
            cout << YELLOW << "Warning: Threshold <= 0 implies always using naive multiplication for base case." << RESET << endl;
        }
        unsigned int num_threads_request = get_valid_input<unsigned int>(BOX_VLINE + string(" Enter number of threads to use (0 for auto): ") + YELLOW); cout << RESET << BOX_VLINE << endl;
        print_footer_box();
        cout << endl;


        // --- 6. Perform Parallel Strassen Multiplication ---
        print_header_box("Performing Multiplication");
        cout << BOX_VLINE << CYAN << " Starting parallel Strassen multiplication..." << RESET << "        " << BOX_VLINE << endl;
        print_footer_box();
        cout << endl;

        // Call the standalone multiplyStrassenParallel
        MultiplicationResult result = multiplyStrassenParallel(A, B, strassen_threshold, num_threads_request);
        cout << GREEN << "\n--- Multiplication Complete ---" << RESET << endl << endl;

        // --- 7. Display Results and Statistics ---
        print_header_box("Results & Statistics");
#ifdef PRINT_MATRICES
        cout << BOX_VLINE << CYAN << " Result Matrix C (First 10x10):" << RESET << "                " << BOX_VLINE << endl;
        print_footer_box(); // Close header box before printing matrix
        result.resultMatrix.print(cout, 3, 10); // Print with scientific notation, limited preview
        print_header_box("End of Matrix Preview"); // Print a box after the matrix
        print_footer_box();
        cout << endl;
#else
        cout << BOX_VLINE << DARK_GRAY << "(Result matrix preview printing is disabled.)" << RESET << "        " << BOX_VLINE << endl;
        print_footer_box();
        cout << endl;
#endif

        print_header_box("Performance Metrics");
        cout << BOX_VLINE << " Final Matrix Dimensions : " << YELLOW << result.resultMatrix.rows() << "x" << result.resultMatrix.cols() << RESET << "                                 " << BOX_VLINE << endl;
        cout << BOX_VLINE << " Total Elements          : " << PURPLE << result.resultMatrix.elementCount() << RESET << "                                 " << BOX_VLINE << endl;
        cout << BOX_LTEE; print_hline(78); cout << BOX_RTEE << endl;
        cout << BOX_VLINE << " Execution Time          : " << GREEN << std::fixed << std::setprecision(4) << result.durationSeconds << " seconds" << RESET << "                        " << BOX_VLINE << endl;
        cout << BOX_VLINE << " CPU Cores Detected      : " << BLUE << result.coresDetected << RESET << "                                 " << BOX_VLINE << endl;
        cout << BOX_VLINE << " Threads Used (Max)      : " << CYAN << result.threadsUsed << RESET << "                                 " << BOX_VLINE << endl;
        cout << BOX_VLINE << " Peak Memory Usage       : " << GREEN << result.memoryInfo.peakWorkingSetMB << " MB" << RESET << "                                 " << BOX_VLINE << endl;
        cout << BOX_VLINE << " Strassen Threshold      : " << YELLOW << result.strassenThreshold << RESET << "                                 " << BOX_VLINE << endl;
        print_footer_box();
        cout << endl;

        // --- 8. Efficiency Note ---
        print_header_box("Efficiency Considerations");
        cout << BOX_VLINE << GREEN << " - " << RESET << "Naive multiplication: O(N^3) time complexity." << "       " << BOX_VLINE << endl;
        cout << BOX_VLINE << GREEN << " - " << RESET << "Strassen's algorithm: Approx O(N^2.807) time." << "       " << BOX_VLINE << endl;
        cout << BOX_VLINE << GREEN << " - " << RESET << "Strassen is faster for large N, due to fewer " << "       " << BOX_VLINE << endl;
        cout << BOX_VLINE << "   multiplications, but has higher overhead (additions, " << "       " << BOX_VLINE << endl;
        cout << BOX_VLINE << "   recursion, memory)." << "                               " << BOX_VLINE << endl;
        cout << BOX_VLINE << GREEN << " - " << RESET << "Parallelism leverages multiple cores to speed up " << "       " << BOX_VLINE << endl;
        cout << BOX_VLINE << "   computation by running sub-problems concurrently." << "       " << BOX_VLINE << endl;
        cout << BOX_VLINE << DARK_GRAY << " - " << RESET << "SIMD (Single Instruction, Multiple Data) instructions" << "  " << BOX_VLINE << endl;
        cout << BOX_VLINE << DARK_GRAY << "   could further optimize naive multiplication for specific " << "  " << BOX_VLINE << endl;
        cout << BOX_VLINE << DARK_GRAY << "   architectures, but require platform-specific code." << "      " << BOX_VLINE << endl;
        print_footer_box();
        cout << endl;


        // --- 9. Save Result ---
        print_header_box("Save Result Matrix");
        char save_choice = get_valid_input<char>(BOX_VLINE + string(" Save result matrix C to file? (y/n): ") + YELLOW); cout << RESET << BOX_VLINE << endl;
        print_footer_box();
        cout << endl;

        if (tolower(save_choice) == 'y') {
            string save_filename = get_valid_input<string>("Enter filename to save result (e.g., result.txt): " + YELLOW); cout << RESET << endl;
            try {
                result.resultMatrix.saveToFile(save_filename);
            }
            catch (const std::exception& e) {
                cerr << RED << "Error saving matrix to file: " << e.what() << RESET << endl;
                // Continue program, don't exit just for failed save
            }
        }


    }
    catch (const std::bad_alloc& e) {
        cerr << "\n" << RED << "*** Memory Allocation Error ***" << RESET << endl;
        cerr << RED << e.what() << RESET << endl;
        cerr << RED << "The requested matrix size is likely too large for the available system memory." << RESET << endl;
        print_footer_box(); // Add footer after error
        return 2;
    }
    catch (const std::exception& e) {
        cerr << "\n" << RED << "*** An error occurred ***" << RESET << endl;
        cerr << RED << e.what() << RESET << endl;
        print_footer_box(); // Add footer after error
        return 1;
    }
    catch (...) {
        cerr << "\n" << RED << "*** An unknown error occurred ***" << RESET << endl;
        print_footer_box(); // Add footer after error
        return 1;
    }

    print_header_box("Program Finished");
    cout << BOX_VLINE << GREEN << " Execution completed successfully." << RESET << "               " << BOX_VLINE << endl;
    print_footer_box();
    cout << endl;

    return 0;
}