// --- Combined Single File: Strassen Matrix Multiplication (Enhanced Version) ---

// Define NOMINMAX *before* including windows.h to prevent macro conflicts
#define NOMINMAX

// --- Includes ---
#include <windows.h> // For Windows API (Memory status, Process info)
#include <psapi.h>   // For GetProcessMemoryInfo
#include <iostream>  // For input/output (cout, cin, cerr)
#include <vector>    // For std::vector (used for matrix data)
#include <string>    // For std::string
#include <thread>    // For std::thread::hardware_concurrency
#include <stdexcept> // For exception classes (std::runtime_error, etc.)
#include <random>    // For random number generation
#include <iomanip>   // For std::setprecision, std::fixed
#include <cmath>     // For std::log2, std::ceil, std::pow
#include <future>    // For std::async, std::future
#include <chrono>    // For timing (high_resolution_clock)
#include <atomic>    // For std::atomic (thread-safe counter)
#include <algorithm> // For std::max, std::min (IMPORTANT!)
#include <limits>    // For std::numeric_limits (used in input clearing, number range)
#include <functional>// For std::ref (used with std::async for references)
#include <string>   //


using std::string; 
using std::cout;
using std::cin;
using std::endl;
using std::cerr;




// Link with Psapi.lib (Specific to MSVC compiler, use -lpsapi for g++)
#pragma comment(lib, "Psapi.lib")

// --- Forward Declarations ---
class Matrix;
struct SystemMemoryInfo;
struct ProcessMemoryInfo;
struct MultiplicationResult;

// --- SystemInfo Implementation ---
struct SystemMemoryInfo {
    unsigned long long totalPhysicalMB;
    unsigned long long availablePhysicalMB;
};

// ::::::::: Colors ::::::::::
const string RED = "\033[1;31m";
const string GREEN = "\033[1;32m";
const string YELLOW = "\033[1;33m";
const string BLUE = "\033[1;34m";
const string PURPLE = "\033[1;35m";
const string CYAN = "\033[1;36m";
const string RESET = "\033[0m";



struct ProcessMemoryInfo {
    size_t peakWorkingSetMB;
};

SystemMemoryInfo getSystemMemoryInfo() {
    MEMORYSTATUSEX statex;
    statex.dwLength = sizeof(statex);
    if (!GlobalMemoryStatusEx(&statex)) {
        DWORD error = GetLastError();
        std::cerr << RED << "Warning: Failed to get system memory status (Error " << error << "). Reporting 0 MB." << std::endl;
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
        std::cerr << RED << "Warning: Failed to get process memory info (Error " << error << "). Reporting 0 MB." << std::endl;
        return { 0 };
    }
}

unsigned long long estimateStrassenMemoryMB(int n) {
    if (n <= 0) return 0;
    unsigned long long elementSize = sizeof(double);
    unsigned long long numElements = static_cast<unsigned long long>(n) * static_cast<unsigned long long>(n);
    unsigned long long matrixSizeBytes = numElements * elementSize;
    unsigned long long estimatedTotalBytes = matrixSizeBytes * 12;
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
        if (numElements > data_.max_size()) throw std::bad_alloc();
        data_.resize(static_cast<size_t>(numElements), 0.0);
    }
    Matrix(int rows, int cols, double initialValue) : rows_(rows), cols_(cols) {
        if (rows < 0 || cols < 0) throw std::invalid_argument("Matrix dimensions cannot be negative.");
        unsigned long long numElements = static_cast<unsigned long long>(rows) * static_cast<unsigned long long>(cols);
        if (numElements > data_.max_size()) throw std::bad_alloc();
        data_.resize(static_cast<size_t>(numElements), initialValue);
    }
    Matrix(const std::vector<std::vector<double>>& data) {
        if (data.empty() || data[0].empty()) { rows_ = 0; cols_ = 0; return; }
        rows_ = static_cast<int>(data.size());
        cols_ = static_cast<int>(data[0].size());
        unsigned long long numElements = static_cast<unsigned long long>(rows_) * static_cast<unsigned long long>(cols_);
        if (numElements > data_.max_size()) throw std::bad_alloc();
        data_.resize(static_cast<size_t>(numElements));
        for (int i = 0; i < rows_; ++i) {
            if (data[i].size() != static_cast<size_t>(cols_)) {
                data_.clear(); rows_ = 0; cols_ = 0;
                throw std::invalid_argument("Inconsistent row lengths in input data.");
            }
            for (int j = 0; j < cols_; ++j) { (*this)(i, j) = data[i][j]; }
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

    // Static functions
    // UPDATED: Generate random doubles within the approximate range of signed 32-bit integers
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

    // Strassen specific helpers
    void split(Matrix& A11, Matrix& A12, Matrix& A21, Matrix& A22) const {
        if (rows_ != cols_ || rows_ % 2 != 0) throw std::runtime_error("Matrix must be square and have even dimensions for splitting.");
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
    static Matrix pad(const Matrix& A, int newSize) {
        if (newSize < A.rows() || newSize < A.cols()) throw std::invalid_argument("New size for padding must be >= original dimensions.");
        Matrix padded(newSize, newSize, 0.0);
        for (int i = 0; i < A.rows(); ++i) for (int j = 0; j < A.cols(); ++j) padded(i, j) = A(i, j);
        return padded;
    }
    static Matrix unpad(const Matrix& A, int originalRows, int originalCols) {
        if (originalRows <= 0 || originalCols <= 0) throw std::invalid_argument("Original dimensions for unpadding must be positive.");
        if (originalRows > A.rows() || originalCols > A.cols()) throw std::invalid_argument("Original dimensions cannot exceed padded dimensions for unpadding.");
        Matrix unpadded(originalRows, originalCols);
        for (int i = 0; i < originalRows; ++i) for (int j = 0; j < originalCols; ++j) unpadded(i, j) = A(i, j);
        return unpadded;
    }

    // Utility
    // Use scientific notation for printing large numbers from random generation
    void print(std::ostream& os = std::cout, int precision = 3) const {
        std::ios_base::fmtflags original_flags = os.flags();
        std::streamsize original_precision = os.precision();

        os << std::scientific << std::setprecision(precision); // Use scientific notation
        os << YELLOW << "Matrix (" << rows_ << "x" << cols_ << "):\n";
        const int max_print_dim = 10;
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
    unsigned int coresDetected; // Added field
    ProcessMemoryInfo memoryInfo;
};

int nextPowerOf2(int n) {
    if (n <= 0) return 1;
    int power = 1;
    while (power < n) {
        power <<= 1;
        if (power <= 0) throw std::overflow_error("Integer overflow calculating next power of 2.");
    }
    return power;
}

Matrix strassen_parallel_internal(const Matrix& A, const Matrix& B, int threshold, int current_depth, int max_depth, std::atomic<unsigned int>& active_threads, unsigned int max_threads);

MultiplicationResult multiplyStrassenParallel(const Matrix& A, const Matrix& B, int threshold = 64, unsigned int num_threads = 0) {
    if (A.cols() != B.rows()) throw std::invalid_argument("Matrix dimensions incompatible (A.cols != B.rows).");
    if (A.isEmpty() || B.isEmpty()) {
        // Get current stats even for empty result
        ProcessMemoryInfo memInfo = getProcessMemoryUsage();
        unsigned int coreCount = getCpuCoreCount();
        return { Matrix(A.rows(), B.cols()), 0.0, 0, coreCount, memInfo };
    }

    unsigned int hardware_cores = getCpuCoreCount();
    unsigned int threads_to_use = (num_threads == 0) ? hardware_cores : std::min(num_threads, hardware_cores);
    if (threads_to_use == 0) threads_to_use = 1;

    int max_depth = (threads_to_use <= 1) ? 0 : static_cast<int>(std::max(1.0, std::log2(static_cast<double>(threads_to_use))));

    int original_rows_A = A.rows();
    int original_cols_B = B.cols();
    int n = std::max({ original_rows_A, A.cols(), B.rows(), original_cols_B });
    int padded_size = nextPowerOf2(n);

    const Matrix Apad = Matrix::pad(A, padded_size);
    const Matrix Bpad = Matrix::pad(B, padded_size);

    std::atomic<unsigned int> active_threads(0);
    auto start_time = std::chrono::high_resolution_clock::now();

    Matrix Cpad = strassen_parallel_internal(Apad, Bpad, threshold, 0, max_depth, active_threads, threads_to_use);

    auto end_time = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> duration = end_time - start_time;

    Matrix C = Matrix::unpad(Cpad, original_rows_A, original_cols_B);
    ProcessMemoryInfo memInfo = getProcessMemoryUsage();

    MultiplicationResult result;
    result.resultMatrix = std::move(C);
    result.durationSeconds = duration.count();
    result.threadsUsed = threads_to_use;
    result.coresDetected = hardware_cores; // Store detected cores
    result.memoryInfo = memInfo;

    return result;
}

Matrix strassen_parallel_internal(const Matrix& A, const Matrix& B, int threshold, int current_depth, int max_depth, std::atomic<unsigned int>& active_threads, unsigned int max_threads)
{
    if (A.rows() <= threshold || A.rows() == 0) {
        if (A.isEmpty() || B.isEmpty()) return Matrix(A.rows(), B.cols());
        return A.multiply_naive(B);
    }
    if (A.rows() != A.cols() || A.rows() != B.rows() || A.rows() % 2 != 0) {
        throw std::logic_error("Internal Error: strassen_parallel_internal called with non-square or odd-dimensioned matrices.");
    }

    int n = A.rows();
    int n2 = n / 2;

    Matrix A11(n2, n2), A12(n2, n2), A21(n2, n2), A22(n2, n2);
    Matrix B11(n2, n2), B12(n2, n2), B21(n2, n2), B22(n2, n2);
    A.split(A11, A12, A21, A22); B.split(B11, B12, B21, B22);

    Matrix S1 = B12 - B22; Matrix S2 = A11 + A12; Matrix S3 = A21 + A22;
    Matrix S4 = B21 - B11; Matrix S5 = A11 + A22; Matrix S6 = B11 + B22;
    Matrix S7 = A12 - A22; Matrix S8 = B21 + B22; Matrix S9 = A21 - A11;
    Matrix S10 = B11 + B12;

    bool launch_async = (current_depth < max_depth);
    Matrix P1, P2, P3, P4, P5, P6, P7;

    if (launch_async) {
        auto future_P1 = std::async(std::launch::async, strassen_parallel_internal, S5, S6, threshold, current_depth + 1, max_depth, std::ref(active_threads), max_threads);
        auto future_P2 = std::async(std::launch::async, strassen_parallel_internal, S3, B11, threshold, current_depth + 1, max_depth, std::ref(active_threads), max_threads);
        auto future_P3 = std::async(std::launch::async, strassen_parallel_internal, A11, S1, threshold, current_depth + 1, max_depth, std::ref(active_threads), max_threads);
        auto future_P4 = std::async(std::launch::async, strassen_parallel_internal, A22, S4, threshold, current_depth + 1, max_depth, std::ref(active_threads), max_threads);
        auto future_P5 = std::async(std::launch::async, strassen_parallel_internal, S2, B22, threshold, current_depth + 1, max_depth, std::ref(active_threads), max_threads);
        auto future_P6 = std::async(std::launch::async, strassen_parallel_internal, S9, S10, threshold, current_depth + 1, max_depth, std::ref(active_threads), max_threads);
        auto future_P7 = std::async(std::launch::async, strassen_parallel_internal, S7, S8, threshold, current_depth + 1, max_depth, std::ref(active_threads), max_threads);

        P1 = future_P1.get(); P2 = future_P2.get(); P3 = future_P3.get();
        P4 = future_P4.get(); P5 = future_P5.get(); P6 = future_P6.get();
        P7 = future_P7.get();
    }
    else {
        P1 = strassen_parallel_internal(S5, S6, threshold, current_depth + 1, max_depth, active_threads, max_threads);
        P2 = strassen_parallel_internal(S3, B11, threshold, current_depth + 1, max_depth, active_threads, max_threads);
        P3 = strassen_parallel_internal(A11, S1, threshold, current_depth + 1, max_depth, active_threads, max_threads);
        P4 = strassen_parallel_internal(A22, S4, threshold, current_depth + 1, max_depth, active_threads, max_threads);
        P5 = strassen_parallel_internal(S2, B22, threshold, current_depth + 1, max_depth, active_threads, max_threads);
        P6 = strassen_parallel_internal(S9, S10, threshold, current_depth + 1, max_depth, active_threads, max_threads);
        P7 = strassen_parallel_internal(S7, S8, threshold, current_depth + 1, max_depth, active_threads, max_threads);
    }

    Matrix C11 = P1 + P4 - P5 + P7; Matrix C12 = P3 + P5;
    Matrix C21 = P2 + P4; Matrix C22 = P1 - P2 + P3 + P6;

    return Matrix::combine(C11, C12, C21, C22);
}


// --- Main Function ---

// --- !!! IMPORTANT !!! ---
// Defining PRINT_MATRICES will print the generated A, B and result C matrices.
// For LARGE matrices (e.g., > 100x100), this will flood the console and take time.
// Comment out the next line to disable printing for large matrix tests.
#define PRINT_MATRICES
// --- !!! IMPORTANT !!! ---

int main() {
    try {
        // --- 1. System Information ---
        std::cout << "--- System Information ---" << std::endl;
        SystemMemoryInfo sysMemInfo = getSystemMemoryInfo();
        unsigned int coreCount = getCpuCoreCount(); // Get core count once
        std::cout << "Total Physical RAM : " << PURPLE << sysMemInfo.totalPhysicalMB << " MB" <<RESET<< endl;
        std::cout << "Available Physical RAM : " << GREEN << sysMemInfo.availablePhysicalMB << GREEN << " MB" <<RESET<<endl;
        std::cout << "Logical CPU Cores detected : " << BLUE << coreCount <<RESET<< std::endl;
        std::cout << "--------------------------" << std::endl << std::endl;

        // --- 2. Get Matrix Dimensions ---
        int rowsA = 0, colsA = 0, rowsB = 0, colsB = 0;
        std::cout << "Enter dimensions for Matrix [ A ] (rows cols): ";
        std::cin >> rowsA >> colsA;
        if (std::cin.fail() || rowsA <= 0 || colsA <= 0) {
            std::cerr << RED << "Error: Invalid input for Matrix A dimensions." <<RESET<< endl;
            std::cin.clear(); std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); return 1;
        }
        std::cout << "Enter dimensions for Matrix [ B ] (rows cols): ";
        std::cin >> rowsB >> colsB;
        if (std::cin.fail() || rowsB <= 0 || colsB <= 0) {
            std::cerr << RED << "Error: Invalid input for Matrix B dimensions." << std::endl;
            std::cin.clear(); std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); return 1;
        }
        if (colsA != rowsB) {
            std::cerr << RED << "Error: Incompatible matrix dimensions (A.cols: " << colsA << " != B.rows: " << rowsB << ")." <<RESET<< std::endl; return 1;
        }

        // --- 3. Estimate Memory ---
        int max_dim = std::max({ rowsA, colsA, rowsB, colsB });
        unsigned long long estimatedMB = estimateStrassenMemoryMB(max_dim);
        std::cout << BLUE<<"Estimated RAM required (approx): " << estimatedMB << " MB" << RESET << std::endl;
        if (sysMemInfo.availablePhysicalMB > 0 && estimatedMB > sysMemInfo.availablePhysicalMB) {
            std::cout << RED << "Warning: Estimated RAM (" << estimatedMB << " MB) exceeds available RAM (" << sysMemInfo.availablePhysicalMB << " MB). Performance may degrade." <<RESET<< std::endl;
        }
        std::cout << "--------------------------" << std::endl << std::endl;

        // --- 4. Generate Matrices ---
        std::cout << CYAN<< "Generating Matrix A (" << rowsA << "x" << colsA << ") and Matrix B (" << rowsB << "x" << colsB
            << ") with values in range [~ -2.147e9, ~ +2.147e9]..." << RESET << std::endl;
        auto gen_start = std::chrono::high_resolution_clock::now();
        // Use the updated generateRandom without min/max args
        Matrix A = Matrix::generateRandom(rowsA, colsA);
        Matrix B = Matrix::generateRandom(rowsB, colsB);
        auto gen_end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> gen_duration = gen_end - gen_start;
        std::cout << "Matrix generation took: " << GREEN << gen_duration.count() << GREEN << " seconds." << std::endl <<RESET<< std::endl;

#ifdef PRINT_MATRICES
        std::cout << CYAN << "--- Generated Matrix A ---" <<RESET<< std::endl;
        A.print(std::cout, 3); // Print with scientific notation
        std::cout << CYAN << "--- Generated Matrix B ---" <<RESET<< std::endl;
        B.print(std::cout, 3); // Print with scientific notation
        std::cout << GREEN << "--- End of Generated Matrices ---" <<RESET<< std::endl << std::endl;
#else
        std::cout << "(Matrix printing is disabled. Undefine or comment out PRINT_MATRICES to enable.)" << std::endl << std::endl;
#endif

        // --- 5. Perform Parallel Strassen Multiplication ---
        std::cout << "--- Performing Parallel Strassen Multiplication ---" << std::endl;
        int strassen_threshold = 64;
        unsigned int num_threads_request = 0; // Auto-detect
        MultiplicationResult result = multiplyStrassenParallel(A, B, strassen_threshold, num_threads_request);
        std::cout << "--- Multiplication Complete ---" << std::endl << std::endl;

        // --- 6. Display Results and Statistics ---
        std::cout << "--- Results & Statistics ---" << std::endl;
#ifdef PRINT_MATRICES
        std::cout << "--- Result Matrix C (" << result.resultMatrix.rows() << "x" << result.resultMatrix.cols() << ") ---" << std::endl;
        result.resultMatrix.print(std::cout, 3); // Print with scientific notation
        std::cout <<RESET<< "--- End of Result Matrix ---" << std::endl << std::endl;
#else
        std::cout << "(Result matrix printing is disabled.)" << std::endl << std::endl;
#endif

        std::cout << "Final Matrix Dimensions: " <<YELLOW<< result.resultMatrix.rows() << "x" << result.resultMatrix.cols() << RESET << std::endl;
        std::cout << "Total Elements in Result: " << PURPLE << result.resultMatrix.elementCount() << RESET << std::endl;
        std::cout << "--------------------------" << std::endl;
        std::cout << "Execution Time: " << GREEN << result.durationSeconds << " seconds" << RESET << std::endl;
        // Enhanced reporting of cores and threads:
        std::cout << "CPU Cores Detected: " << BLUE << result.coresDetected << RESET << std::endl;
        std::cout << "Threads Used (Max Allowed): " << CYAN << result.threadsUsed << RESET << std::endl;
        std::cout << "Peak Memory Usage (Process Working Set): " << GREEN << result.memoryInfo.peakWorkingSetMB << RESET << " MB" << std::endl;
        std::cout << "Strassen Threshold (Switch to Naive): " <<YELLOW<< strassen_threshold << RESET << std::endl;
        std::cout << "--------------------------" << std::endl;

        // --- 7. Efficiency Note ---
        cout << "Efficiency Note:" << std::endl;
        cout  <<GREEN << " - "  << RESET  << "Naive multiplication has a time complexity of O(N^3)." << std::endl;
        cout << GREEN << " - " << RESET << "Strassen's algorithm reduces this to approximately O(N^2.807)." << std::endl;
        cout << GREEN << " - " << RESET << "For large matrices, Strassen's algorithm performs significantly fewer arithmetic operations." << std::endl;
        cout << GREEN << " - " << RESET << "However, Strassen's has higher overhead (more additions/subtractions, recursion), "
            << "so the naive method is often faster for small matrices (below the threshold)." << std::endl;
        cout << GREEN << " - " << RESET << " Parallelism further speeds up calculations on multi-core CPUs by executing sub-problems concurrently." << std::endl;
        cout << "--------------------------" << std::endl;


    }
    catch (const std::bad_alloc& e) {
        cerr << "\n *** Memory Allocation Error ***\n " << e.what() << "\n The requested matrix size is likely too large for the available system memory." << std::endl; return 2;
    }
    catch (const std::exception& e) {
        cerr << "\n *** An error occurred ***\n " << e.what() << std::endl; return 1;
    }
    catch (...) {
        cerr << "\n *** An unknown error occurred ***" << std::endl; return 1;
    }
    return 0;
}