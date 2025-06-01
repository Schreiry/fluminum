
// Define NOMINMAX *before* including windows.h to prevent macro conflicts
#define NOMINMAX

// --- Includes ---
#include <windows.h> // For Windows API (Memory status, Process info, SetConsoleOutputCP, QueryPerformanceCounter/Frequency, Beep)
#include <psapi.h>   // For GetProcessMemoryInfo
#include <iostream>  // For input/output (cout, cin, cerr)
#include <vector>    // For std::vector (used for matrix data)
#include <string>    // For std::string
#include <thread>    // For std::thread, std::thread::hardware_concurrency, std::this_thread::sleep_for
#include <stdexcept> // For exception classes (std::runtime_error, etc.)
#include <random>    // For random number generation
#include <iomanip>   // For std::setprecision, std::fixed, std::setw
#include <cmath>     // For std::log2, std::ceil, std::pow, std::abs
#include <future>    // For std::async, std::future
#include <chrono>    // For timing (high_resolution_clock)
#include <atomic>    // For std::atomic (thread-safe counter for progress)
#include <algorithm> // For std::max, std::min, std::swap, std::abs
#include <limits>    // For std::numeric_limits (used in input clearing, number range)
#include <functional>// For std::ref (used with std::async for references)
#include <fstream>   // For file input/output (std::ifstream, std::ofstream)
#include <sstream>   // For string stream processing (std::stringstream)
#include <cctype>    // For tolower, isspace
#include <type_traits> // For std::is_same

// Include for SIMD intrinsics (Windows, for Intel/AMD processors supporting AVX)
#ifdef _MSC_VER
#include <intrin.h> // For __cpuidex and _xgetbv
#endif
#if defined(__AVX__) || (defined(_MSC_VER) && defined(__AVX__))
#include <immintrin.h> // AVX intrinsics
#define HAS_AVX
const int SIMD_VECTOR_SIZE_DOUBLE = 4; // AVX processes 4 doubles at once
#elif defined(__SSE2__) || (defined(_MSC_VER) && defined(__SSE2__))
#include <emmintrin.h> // SSE2 intrinsics
#define HAS_SSE2
const int SIMD_VECTOR_SIZE_DOUBLE = 2; // SSE2 processes 2 doubles at once
#else
const int SIMD_VECTOR_SIZE_DOUBLE = 1; // Scalar processing
#endif


// Link with Psapi.lib (Specific to MSVC compiler, use -lpsapi for g++)
#pragma comment(lib, "Psapi.lib")

// Using directives for common elements
using std::string;
using std::cout;
using std::cin;
using std::endl;
using std::cerr;


// --- Console Formatting ---
const string RED = "\033[1;31m";
const string GREEN = "\033[1;32m";
const string YELLOW = "\033[1;33m";
const string BLUE = "\033[1;34m";
const string PURPLE = "\033[1;35m";
const string CYAN = "\033[1;36m";
const string RESET = "\033[0m";
const string DARK_GRAY = "\033[0;90m";
const string LIGHT_GRAY = "\033[0;37m";

const string SPINNER_CHARS[] = { CYAN + "|" + RESET, YELLOW + "/" + RESET, BLUE + "-" + RESET, PURPLE + "\\" + RESET };
const int NUM_SPINNER_CHARS = 4;

const string BOX_HLINE = "\u2500";
const string BOX_VLINE = "\u2502";
const string BOX_TLCORNER = "\u250C";
const string BOX_TRCORNER = "\u2510";
const string BOX_BLCORNER = "\u2514";
const string BOX_BRCORNER = "\u2518";
const string BOX_LTEE = "\u251C"; // Stem on left, bar to right (|- )
const string BOX_RTEE = "\u2524"; // Stem on right, bar to left ( -|)
const string BOX_BTEE = "\u2534";
const string BOX_TTEE = "\u252C";
const string BOX_CROSS = "\u253C";

enum class Alignment { Left, Center, Right };

int get_visible_width(const std::string& text) {
    int visible_width = 0;
    bool in_escape_sequence = false;
    for (char c : text) {
        if (in_escape_sequence) {
            if (c == 'm') {
                in_escape_sequence = false;
            }
        }
        else if (c == '\033') {
            in_escape_sequence = true;
        }
        else {
            visible_width++;
        }
    }
    return visible_width;
}

void print_hline(int width) {
    for (int i = 0; i < width; ++i) cout << BOX_HLINE;
}

void print_header_box(const string& title, int width = 80) {
    int title_visible_len = get_visible_width(title);
    int padding_total = width - title_visible_len - 4;
    int padding_left = padding_total / 2;
    int padding_right = padding_total - padding_left;

    if (padding_left < 0 || padding_right < 0) {
        padding_left = 0;
        padding_right = 0;
    }

    cout << BOX_TLCORNER; print_hline(width - 2); cout << BOX_TRCORNER << endl;
    cout << BOX_VLINE << std::string(padding_left > 0 ? padding_left : 0, ' ') << " " << title << " " << std::string(padding_right > 0 ? padding_right : 0, ' ') << RESET << BOX_VLINE << endl;
    cout << BOX_LTEE; print_hline(width - 2); cout << BOX_RTEE << endl;
}

void print_footer_box(int width = 80) {
    cout << BOX_BLCORNER; print_hline(width - 2); cout << BOX_BRCORNER << endl;
}

void print_separator_line(int width = 80) {
    cout << BOX_LTEE; print_hline(width - 2); cout << BOX_RTEE << endl;
}

void print_line_in_box(const std::string& content, int width = 80, bool add_color_reset_at_end = true, Alignment alignment = Alignment::Left) {
    int box_content_width = width - 2;
    int content_visible_width = get_visible_width(content);
    int padding = box_content_width - content_visible_width;

    if (padding < 0) {
        padding = 0; // Content is wider, will overflow but no negative padding
    }

    cout << BOX_VLINE;

    string processed_content = content;

    if (alignment == Alignment::Left) {
        cout << processed_content << std::string(padding, ' ');
    }
    else if (alignment == Alignment::Right) {
        cout << std::string(padding, ' ') << processed_content;
    }
    else if (alignment == Alignment::Center) {
        int padding_left = padding / 2;
        int padding_right = padding - padding_left;
        cout << std::string(padding_left, ' ') << processed_content << std::string(padding_right, ' ');
    }

    if (add_color_reset_at_end) {
        if (!(processed_content.length() >= RESET.length() &&
            processed_content.substr(processed_content.length() - RESET.length()) == RESET)) {
            cout << RESET;
        }
    }
    cout << BOX_VLINE << endl;
}

// --- Loading Animation and Sound ---
void show_loading_animation_step(int& spinner_idx, const std::string& message) {
    string display_message = message;
    if (get_visible_width(message) > 60) {
        display_message = message.substr(0, 57) + "...";
    }
    cout << "\r" << display_message << " " << SPINNER_CHARS[spinner_idx] << " " << std::flush;
    spinner_idx = (spinner_idx + 1) % NUM_SPINNER_CHARS;
}

void play_completion_sound() {
#ifdef _WIN32
    Beep(623, 150); // D#5
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    Beep(783, 200); // G5
#else
    cout << '\a' << std::flush; // Standard terminal bell
#endif
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

LARGE_INTEGER g_performanceFrequency = { 0 };

void initializePerformanceCounter() {
    if (!QueryPerformanceFrequency(&g_performanceFrequency)) {
        cerr << RED << "Warning: QueryPerformanceFrequency failed. High-resolution timing may be unavailable." << RESET << endl;
        g_performanceFrequency.QuadPart = 0;
    }
}

bool has_avx = false;
bool has_sse2 = false;

void check_simd_support() {
#ifdef _MSC_VER
    int cpuInfo[4];
    __cpuidex(cpuInfo, 1, 0);
    has_sse2 = (cpuInfo[3] & (1 << 26)) != 0;

    bool osxsave_supported = (cpuInfo[2] & (1 << 27)) != 0;
    bool avx_cpu_supported = (cpuInfo[2] & (1 << 28)) != 0;

    has_avx = false;
    if (avx_cpu_supported && osxsave_supported) {
        unsigned __int64 xcrFeatureMask = _xgetbv(0);
        if ((xcrFeatureMask & 6) == 6) {
            has_avx = true;
        }
    }
#elif defined(__GNUC__) || defined(__clang__)
#ifdef __AVX__
    has_avx = true;
#endif
#ifdef __SSE2__
    has_sse2 = true;
#endif
#else
    has_avx = false;
    has_sse2 = false;
#endif
}

unsigned long long estimateStrassenMemoryMB(int n_padded) {
    if (n_padded <= 0) return 0;
    unsigned long long elementSize = sizeof(double);
    unsigned long long numElementsPerPaddedMatrix = static_cast<unsigned long long>(n_padded) * static_cast<unsigned long long>(n_padded);
    unsigned long long estimatedTotalElements = numElementsPerPaddedMatrix * 18;
    unsigned long long estimatedTotalBytes = estimatedTotalElements * elementSize;
    return estimatedTotalBytes / (1024 * 1024);
}

unsigned long long estimateComparisonMemoryMB(int n_padded) {
    if (n_padded <= 0) return 0;
    unsigned long long elementSize = sizeof(double);
    unsigned long long numElementsPerPaddedMatrix = static_cast<unsigned long long>(n_padded) * static_cast<unsigned long long>(n_padded);
    unsigned long long estimatedTotalElements = numElementsPerPaddedMatrix * 3;
    unsigned long long estimatedTotalBytes = estimatedTotalElements * elementSize;
    return estimatedTotalBytes / (1024 * 1024);
}


// --- Matrix Class Implementation ---
class Matrix {
public:
    Matrix() : rows_(0), cols_(0) {}
    Matrix(int rows, int cols) : rows_(rows), cols_(cols) {
        if (rows < 0 || cols < 0) throw std::invalid_argument("Matrix dimensions cannot be negative.");
        unsigned long long numElements_ull = static_cast<unsigned long long>(rows) * static_cast<unsigned long long>(cols);
        if (rows > 0 && cols > 0 && numElements_ull / static_cast<unsigned long long>(rows) != static_cast<unsigned long long>(cols)) {
            throw std::bad_alloc();
        }
        if (numElements_ull > std::vector<double>().max_size()) {
            throw std::bad_alloc();
        }
        data_.resize(static_cast<size_t>(numElements_ull), 0.0);
    }
    Matrix(int rows, int cols, double initialValue) : rows_(rows), cols_(cols) {
        if (rows < 0 || cols < 0) throw std::invalid_argument("Matrix dimensions cannot be negative.");
        unsigned long long numElements_ull = static_cast<unsigned long long>(rows) * static_cast<unsigned long long>(cols);
        if (rows > 0 && cols > 0 && numElements_ull / static_cast<unsigned long long>(rows) != static_cast<unsigned long long>(cols)) {
            throw std::bad_alloc();
        }
        if (numElements_ull > std::vector<double>().max_size()) {
            throw std::bad_alloc();
        }
        data_.resize(static_cast<size_t>(numElements_ull), initialValue);
    }
    Matrix(const std::vector<std::vector<double>>& data_2d) {
        if (data_2d.empty()) {
            rows_ = 0; cols_ = 0;
            data_.clear();
            return;
        }
        rows_ = static_cast<int>(data_2d.size());
        cols_ = static_cast<int>(data_2d[0].size());
        for (size_t i = 1; i < data_2d.size(); ++i) {
            if (data_2d[i].size() != static_cast<size_t>(cols_)) {
                rows_ = 0; cols_ = 0; data_.clear();
                throw std::invalid_argument("Inconsistent row lengths in input data for Matrix constructor.");
            }
        }

        unsigned long long numElements_ull = static_cast<unsigned long long>(rows_) * static_cast<unsigned long long>(cols_);
        if (rows_ > 0 && cols_ > 0 && numElements_ull / static_cast<unsigned long long>(rows_) != static_cast<unsigned long long>(cols_)) {
            throw std::bad_alloc();
        }
        if (numElements_ull > std::vector<double>().max_size()) {
            throw std::bad_alloc();
        }
        data_.resize(static_cast<size_t>(numElements_ull));

        for (int i = 0; i < rows_; ++i) {
            for (int j = 0; j < cols_; ++j) {
                (*this)(i, j) = data_2d[i][j];
            }
        }
    }

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
        if (rows_ == 0 || cols_ == 0 || other.cols_ == 0) return Matrix(rows_, other.cols_);

        Matrix result(rows_, other.cols_);
        int M = rows_; int N = cols_; int P = other.cols_;
        const double* a_ptr = data_.data();
        const double* b_ptr = other.data_.data();
        double* c_ptr = result.data_.data();

#ifdef HAS_AVX
        if (has_avx && N > 0 && P >= SIMD_VECTOR_SIZE_DOUBLE) {
            int P_aligned = P - (P % SIMD_VECTOR_SIZE_DOUBLE);
            for (int i = 0; i < M; ++i) {
                for (int j = 0; j < P_aligned; j += SIMD_VECTOR_SIZE_DOUBLE) {
                    __m256d c_vec = _mm256_setzero_pd();
                    for (int k = 0; k < N; ++k) {
                        __m256d a_scalar = _mm256_broadcast_sd(a_ptr + static_cast<size_t>(i) * N + k);
                        __m256d b_vec = _mm256_loadu_pd(b_ptr + static_cast<size_t>(k) * P + j);
                        c_vec = _mm256_add_pd(c_vec, _mm256_mul_pd(a_scalar, b_vec));
                    }
                    _mm256_storeu_pd(c_ptr + static_cast<size_t>(i) * P + j, c_vec);
                }
                for (int j = P_aligned; j < P; ++j) {
                    double sum = 0.0;
                    for (int k = 0; k < N; ++k) sum += a_ptr[static_cast<size_t>(i) * N + k] * b_ptr[static_cast<size_t>(k) * P + j];
                    c_ptr[static_cast<size_t>(i) * P + j] = sum;
                }
            }
            return result;
        }
#endif
#ifdef HAS_SSE2
        if (has_sse2 && N > 0 && P >= SIMD_VECTOR_SIZE_DOUBLE && !has_avx) {
            int P_aligned = P - (P % SIMD_VECTOR_SIZE_DOUBLE);
            for (int i = 0; i < M; ++i) {
                for (int j = 0; j < P_aligned; j += SIMD_VECTOR_SIZE_DOUBLE) {
                    __m128d c_vec = _mm_setzero_pd();
                    for (int k = 0; k < N; ++k) {
                        __m128d a_scalar = _mm_load_sd(a_ptr + static_cast<size_t>(i) * N + k);
                        a_scalar = _mm_unpacklo_pd(a_scalar, a_scalar);
                        __m128d b_vec = _mm_loadu_pd(b_ptr + static_cast<size_t>(k) * P + j);
                        c_vec = _mm_add_pd(c_vec, _mm_mul_pd(a_scalar, b_vec));
                    }
                    _mm_storeu_pd(c_ptr + static_cast<size_t>(i) * P + j, c_vec);
                }
                for (int j = P_aligned; j < P; ++j) {
                    double sum = 0.0;
                    for (int k = 0; k < N; ++k) sum += a_ptr[static_cast<size_t>(i) * N + k] * b_ptr[static_cast<size_t>(k) * P + j];
                    c_ptr[static_cast<size_t>(i) * P + j] = sum;
                }
            }
            return result;
        }
#endif
        for (int i = 0; i < M; ++i) {
            for (int j = 0; j < P; ++j) {
                double sum = 0.0;
                for (int k = 0; k < N; ++k) sum += (*this)(i, k) * other(k, j);
                result(i, j) = sum;
            }
        }
        return result;
    }

    long long compare_naive(const Matrix& other, double epsilon = 0.0) const {
        if (rows_ != other.rows_ || cols_ != other.cols_) {
            throw std::invalid_argument("Matrix dimensions must match for comparison.");
        }
        if (rows_ == 0 || cols_ == 0) return 0;
        long long match_count = 0;
        if (epsilon > 0) {
            for (size_t i = 0; i < data_.size(); ++i) {
                if (std::abs(data_[i] - other.data_[i]) <= epsilon) match_count++;
            }
        }
        else {
            for (size_t i = 0; i < data_.size(); ++i) {
                if (data_[i] == other.data_[i]) match_count++;
            }
        }
        return match_count;
    }

    static Matrix generateRandom(int rows, int cols) {
        if (rows < 0 || cols < 0) throw std::invalid_argument("Matrix dimensions cannot be negative for random generation.");
        if (rows == 0 || cols == 0) return Matrix(rows, cols);

        constexpr double minVal = -10.0;
        constexpr double maxVal = 10.0;

        Matrix result(rows, cols);
        std::random_device rd;
        std::mt19937 gen(rd());
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

    static Matrix readFromConsole(int rows, int cols, int& /*spinner_idx unused here*/) {
        if (rows <= 0 || cols <= 0) throw std::invalid_argument("Matrix dimensions must be positive for console input.");
        Matrix result(rows, cols);
        cout << CYAN << "Enter elements for the " << rows << "x" << cols << " matrix:" << RESET << endl;
        cout << DARK_GRAY << "(Space/comma separated numbers per row, Enter after each row)" << RESET << endl;

        for (int i = 0; i < rows; ++i) {
            bool row_ok = false;
            while (!row_ok) {
                cout << "Row " << (i + 1) << "/" << rows << ": " << YELLOW;
                string line;
                if (!std::getline(std::cin >> std::ws, line)) {
                    cout << RESET;
                    if (std::cin.eof()) throw std::runtime_error("EOF reached during console input.");
                    else throw std::runtime_error("Failed to read line from console.");
                }
                cout << RESET;

                std::stringstream ss(line);
                double val;
                int j = 0;
                char comma;

                while (j < cols && ss >> val) {
                    result(i, j) = val;
                    j++;
                    ss >> std::ws;
                    if (ss.peek() == ',') ss >> comma;
                    ss >> std::ws;
                }

                if (j == cols && (ss.eof() || ss.str().substr(ss.tellg()).find_first_not_of(" \t\n\v\f\r") == string::npos)) {
                    row_ok = true;
                }
                else {
                    cerr << RED << "\nError reading row " << (i + 1) << ". Expected " << cols << " numbers. Read " << j << ". Remainder: '" << (ss.eof() ? "<EOF>" : ss.str().substr(ss.tellg())) << "'. Try again." << RESET << endl;
                }
            }
        }
        cout << GREEN << "Matrix input complete." << RESET << endl;
        return result;
    }

    static Matrix readFromFile(const std::string& filename) {
        std::ifstream infile(filename);
        if (!infile.is_open()) throw std::runtime_error("Could not open file: " + filename);

        std::vector<std::vector<double>> temp_data;
        string line;
        int expected_cols = -1;
        int current_row_num = 0;
        int spinner_idx = 0;

        cout << CYAN << "Reading matrix from file: " << filename << RESET;
        auto last_update_time = std::chrono::steady_clock::now();

        while (std::getline(infile, line)) {
            current_row_num++;
            if (line.find_first_not_of(" \t\n\v\f\r") == string::npos) continue;

            std::vector<double> row_vec;
            std::stringstream ss(line);
            double val;
            char comma;

            while (ss >> val) {
                row_vec.push_back(val);
                ss >> std::ws;
                if (ss.peek() == ',') ss >> comma;
                ss >> std::ws;
            }

            string remaining_in_ss;
            if (ss >> remaining_in_ss && remaining_in_ss.find_first_not_of(" \t\n\v\f\r") != string::npos) {
                cout << "\r" << string(80, ' ') << "\r"; infile.close();
                throw std::invalid_argument("Malformed data in file " + filename + " at row " + std::to_string(current_row_num) + ". Extra characters: '" + remaining_in_ss + "'");
            }

            if (expected_cols == -1) {
                expected_cols = static_cast<int>(row_vec.size());
                if (expected_cols == 0 && !line.empty() && line.find_first_not_of(", \t") != std::string::npos) {
                    cout << "\r" << string(80, ' ') << "\r"; infile.close();
                    throw std::invalid_argument("First data row in " + filename + " appears to contain no valid numbers.");
                }
            }
            else if (static_cast<int>(row_vec.size()) != expected_cols) {
                cout << "\r" << string(80, ' ') << "\r"; infile.close();
                throw std::invalid_argument("Inconsistent columns in " + filename + " at row " + std::to_string(current_row_num) + ". Expected " + std::to_string(expected_cols) + ", got " + std::to_string(row_vec.size()));
            }

            if (expected_cols == 0 || static_cast<int>(row_vec.size()) == expected_cols) {
                temp_data.push_back(row_vec);
            }

            auto current_time = std::chrono::steady_clock::now();
            if (std::chrono::duration_cast<std::chrono::milliseconds>(current_time - last_update_time).count() > 100) {
                show_loading_animation_step(spinner_idx, "Reading rows: " + std::to_string(temp_data.size()));
                last_update_time = current_time;
            }
        }
        cout << "\r" << string(80, ' ') << "\r";

        infile.close();

        if (temp_data.empty() && expected_cols == -1) {
            cout << YELLOW << "Warning: File '" << filename << "' empty or no valid data. Creating 0x0 matrix." << RESET << endl;
            return Matrix(0, 0);
        }
        if (expected_cols == -1) expected_cols = 0;

        cout << GREEN << "Successfully read " << temp_data.size() << " data rows, expecting " << expected_cols << " columns, from file." << RESET << endl;
        return Matrix(temp_data);
    }

    void saveToFile(const std::string& filename, char separator = ' ') const {
        std::ofstream outfile(filename);
        if (!outfile.is_open()) throw std::runtime_error("Could not open file for writing: " + filename);

        cout << CYAN << "Saving matrix to file: " << filename << RESET << endl;
        outfile << std::scientific << std::setprecision(10);

        for (int i = 0; i < rows_; ++i) {
            for (int j = 0; j < cols_; ++j) {
                outfile << (*this)(i, j) << (j == cols_ - 1 ? "" : std::string(1, separator));
            }
            outfile << endl;
        }
        outfile.close();
        if (outfile.fail()) cerr << RED << "Error encountered while writing or closing file: " << filename << RESET << endl;
        else cout << GREEN << "Matrix successfully saved to " << filename << RESET << endl;
    }

    void split(Matrix& A11, Matrix& A12, Matrix& A21, Matrix& A22) const {
        if (rows_ != cols_ || rows_ % 2 != 0 || rows_ == 0) throw std::logic_error("Internal Error: Matrix for split must be non-empty, square, and even-dimensioned.");
        int n2 = rows_ / 2;
        A11 = Matrix(n2, n2); A12 = Matrix(n2, n2); A21 = Matrix(n2, n2); A22 = Matrix(n2, n2);
        for (int i = 0; i < n2; ++i) {
            for (int j = 0; j < n2; ++j) {
                A11(i, j) = (*this)(i, j); A12(i, j) = (*this)(i, j + n2);
                A21(i, j) = (*this)(i + n2, j); A22(i, j) = (*this)(i + n2, j + n2);
            }
        }
    }

    static void split(const Matrix& A, const Matrix& B,
        Matrix& A11, Matrix& A12, Matrix& A21, Matrix& A22,
        Matrix& B11, Matrix& B12, Matrix& B21, Matrix& B22) {
        if (A.rows() != A.cols() || A.rows() != B.rows() || B.rows() != B.cols() || A.rows() % 2 != 0 || A.rows() == 0) {
            throw std::logic_error("Internal Error: Matrices for split must be non-empty, square, same even dimensions.");
        }
        int n2 = A.rows() / 2;
        A11 = Matrix(n2, n2); A12 = Matrix(n2, n2); A21 = Matrix(n2, n2); A22 = Matrix(n2, n2);
        B11 = Matrix(n2, n2); B12 = Matrix(n2, n2); B21 = Matrix(n2, n2); B22 = Matrix(n2, n2);

        for (int i = 0; i < n2; ++i) {
            for (int j = 0; j < n2; ++j) {
                A11(i, j) = A(i, j);       A12(i, j) = A(i, j + n2);
                A21(i, j) = A(i + n2, j); A22(i, j) = A(i + n2, j + n2);
                B11(i, j) = B(i, j);       B12(i, j) = B(i, j + n2);
                B21(i, j) = B(i + n2, j); B22(i, j) = B(i + n2, j + n2);
            }
        }
    }

    static Matrix combine(const Matrix& C11, const Matrix& C12, const Matrix& C21, const Matrix& C22) {
        int n2 = C11.rows();
        if (C11.cols() != n2 || C12.rows() != n2 || C12.cols() != n2 ||
            C21.rows() != n2 || C21.cols() != n2 || C22.rows() != n2 || C22.cols() != n2 || n2 == 0)
            throw std::invalid_argument("Quadrants for combining must be non-empty, square, and same dimensions.");
        int n = n2 * 2;
        Matrix C(n, n);
        for (int i = 0; i < n2; ++i) {
            for (int j = 0; j < n2; ++j) {
                C(i, j) = C11(i, j);        C(i, j + n2) = C12(i, j);
                C(i + n2, j) = C21(i, j); C(i + n2, j + n2) = C22(i, j);
            }
        }
        return C;
    }

    static Matrix pad(const Matrix& A, int targetSize) {
        if (targetSize < A.rows() || targetSize < A.cols()) throw std::invalid_argument("Target size for padding must be >= original dimensions.");
        if (targetSize == A.rows() && targetSize == A.cols()) return A;
        if (targetSize <= 0 && (A.rows() == 0 || A.cols() == 0)) return Matrix(0, 0);
        if (targetSize <= 0) throw std::invalid_argument("Target size for padding non-empty matrix must be positive.");

        Matrix padded(targetSize, targetSize, 0.0);
        for (int i = 0; i < A.rows(); ++i) {
            for (int j = 0; j < A.cols(); ++j) padded(i, j) = A(i, j);
        }
        return padded;
    }

    static Matrix unpad(const Matrix& A, int originalRows, int originalCols) {
        if (originalRows == A.rows() && originalCols == A.cols()) return A;
        if (originalRows == 0 || originalCols == 0) {
            if (originalRows == 0 && originalCols == 0) return Matrix(0, 0);
            if (originalRows > 0 && originalCols != 0) throw std::invalid_argument("Cannot unpad to M x N if N is not 0.");
            if (originalCols > 0 && originalRows != 0) throw std::invalid_argument("Cannot unpad to M x N if M is not 0.");
            return Matrix(originalRows, originalCols);
        }
        if (originalRows <= 0 || originalCols <= 0) {
            throw std::invalid_argument("Original dimensions for unpadding must be positive (unless a 0 dimension).");
        }
        if (originalRows > A.rows() || originalCols > A.cols()) throw std::invalid_argument("Original dimensions exceed padded dimensions for unpadding.");

        Matrix unpadded(originalRows, originalCols);
        for (int i = 0; i < originalRows; ++i) {
            for (int j = 0; j < originalCols; ++j) unpadded(i, j) = A(i, j);
        }
        return unpadded;
    }

    void print(std::ostream& os = std::cout, int precision = 3, int max_print_dim = 10) const {
        std::ios_base::fmtflags original_flags = os.flags();
        std::streamsize original_precision = os.precision();
        os << std::fixed << std::setprecision(precision);

        os << YELLOW << "Matrix (" << rows_ << "x" << cols_ << "):" << RESET << "\n";
        if (isEmpty()) {
            os << DARK_GRAY << "(Empty Matrix)" << RESET << "\n" << endl;
            os.flags(original_flags); os.precision(original_precision);
            return;
        }

        int print_rows = std::min(rows_, max_print_dim);
        int print_cols = std::min(cols_, max_print_dim);

        for (int i = 0; i < print_rows; ++i) {
            os << "[ ";
            for (int j = 0; j < print_cols; ++j) {
                os << std::setw(precision + 5) << (*this)(i, j) << (j == print_cols - 1 ? "" : " ");
            }
            if (cols_ > print_cols) os << " ... ";
            os << " ]\n";
        }
        if (rows_ > print_rows) os << "  ...\n";
        os << std::endl;

        os.flags(original_flags);
        os.precision(original_precision);
    }
    bool isEmpty() const { return rows_ == 0 || cols_ == 0; }
    size_t elementCount() const { return static_cast<size_t>(rows_) * static_cast<size_t>(cols_); }

private:
    int rows_;
    int cols_;
    std::vector<double> data_;
};


// --- Helper for power of 2 ---
int nextPowerOf2(int n) {
    if (n <= 0) return 1;
    if (n == 1) return 1;

    unsigned long long n_ull = static_cast<unsigned long long>(n);
    if (n > 32768) {
        unsigned long long elements_ll_est = (n_ull > std::numeric_limits<unsigned long long>::max() / n_ull)
            ? std::numeric_limits<unsigned long long>::max() : n_ull * n_ull;

        SystemMemoryInfo tempSysMem = getSystemMemoryInfo();
        unsigned long long max_practical_elements = (tempSysMem.totalPhysicalMB > 0) ? (tempSysMem.totalPhysicalMB * 1024 * 1024 / (3 * sizeof(double))) : std::vector<double>().max_size();

        if (elements_ll_est > std::vector<double>().max_size() || (elements_ll_est > max_practical_elements && max_practical_elements > 0 && elements_ll_est > 100000000)) {
            throw std::overflow_error("Input dimension " + std::to_string(n) + " is impractically large for padding to next power of 2.");
        }
    }

    int power = 1;
    while (power < n) {
        if (power > std::numeric_limits<int>::max() / 2) {
            throw std::overflow_error("Cannot compute next power of 2 for " + std::to_string(n) + " without integer overflow.");
        }
        power *= 2;
    }
    return power;
}

// --- NEW: Progress Bar Implementation ---

// Calculates the total number of base-case (naive) multiplications.
long long calculate_total_tasks(int n, int threshold) {
    if (n <= 0) return 0;
    // Use effective threshold of 1 if user specified 0, for calculation.
    int eff_threshold = (threshold == 0) ? 1 : threshold;
    if (n <= eff_threshold) return 1LL;
    // Strassen performs 7 recursive calls on matrices of size n/2.
    return 7LL * calculate_total_tasks(n / 2, threshold);
}

// Displays the progress bar in a separate thread.
void display_progress(std::atomic<int>& counter, long long total, std::atomic<bool>& done) {
    int last_percent = -1;
    auto start_time = std::chrono::steady_clock::now();

    while (!done.load(std::memory_order_acquire)) {
        int current_count = counter.load(std::memory_order_acquire);
        int percent = (total > 0) ? static_cast<int>((static_cast<double>(current_count) / total) * 100.0) : 100;
        percent = std::min(100, percent); // Cap at 100

        if (percent > last_percent || percent == 0) {
            cout << "\r" << YELLOW << "Progress: [" << GREEN;
            int bar_width = 30;
            int pos = (bar_width * percent) / 100;
            for (int i = 0; i < bar_width; ++i) {
                cout << (i < pos ? '#' : '-');
            }
            cout << YELLOW << "] " << std::setw(3) << percent << "% (" << current_count << "/" << total << ")" << RESET << std::flush;
            last_percent = percent;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(150)); // Update ~6 times/sec
    }

    // Ensure the final 100% is displayed.
    auto end_time = std::chrono::steady_clock::now();
    double elapsed_sec = std::chrono::duration<double>(end_time - start_time).count();
    cout << "\r" << YELLOW << "Progress: [" << GREEN << std::string(30, '#') << YELLOW << "] 100% (" << total << "/" << total << ") "
        << GREEN << "Done in " << std::fixed << std::setprecision(2) << elapsed_sec << "s." << RESET << std::string(10, ' ') << std::flush;
    cout << endl; // New line after completion
}


// --- StrassenMultiply Implementation ---
// Forward Declaration (Updated Signature)
Matrix strassen_recursive_worker(Matrix A, Matrix B, int threshold, int current_depth, int max_depth_async, std::atomic<int>& progress_counter);

struct MultiplicationResult {
    Matrix resultMatrix;
    double durationSeconds_chrono;
    long long durationNanoseconds_chrono;
    double durationSeconds_qpc;
    unsigned int threadsUsed;
    unsigned int coresDetected;
    ProcessMemoryInfo memoryInfo;
    int strassenThreshold;
    int originalRowsA, originalColsA, originalRowsB, originalColsB;

    double padding_duration_sec = 0.0;
    double unpadding_duration_sec = 0.0;
    double first_level_split_sec = 0.0;
    double first_level_S_calc_sec = 0.0;
    double first_level_P_tasks_wall_sec = 0.0;
    double first_level_C_quad_calc_sec = 0.0;
    double first_level_final_combine_sec = 0.0;
    bool strassen_applied_at_top_level = false;

    MultiplicationResult() :
        resultMatrix(0, 0), durationSeconds_chrono(0.0), durationNanoseconds_chrono(0LL),
        durationSeconds_qpc(0.0), threadsUsed(0), coresDetected(0),
        memoryInfo({ 0 }), strassenThreshold(0),
        originalRowsA(0), originalColsA(0), originalRowsB(0), originalColsB(0) {
    }
};


MultiplicationResult multiplyStrassenParallel(const Matrix& A_orig, const Matrix& B_orig, int threshold, unsigned int num_threads_request = 0) {
    MultiplicationResult result_obj;
    result_obj.originalRowsA = A_orig.rows();
    result_obj.originalColsA = A_orig.cols();
    result_obj.originalRowsB = B_orig.rows();
    result_obj.originalColsB = B_orig.cols();
    result_obj.strassenThreshold = threshold;

    if (A_orig.cols() != B_orig.rows()) throw std::invalid_argument("Matrix dimensions incompatible (A.cols != B.rows).");

    if (A_orig.isEmpty() || B_orig.isEmpty() || A_orig.cols() == 0) {
        result_obj.resultMatrix = Matrix(A_orig.rows(), B_orig.cols());
        result_obj.memoryInfo = getProcessMemoryUsage();
        result_obj.coresDetected = getCpuCoreCount();
        return result_obj;
    }

    unsigned int hardware_cores = getCpuCoreCount();
    result_obj.coresDetected = hardware_cores;
    result_obj.threadsUsed = (num_threads_request == 0) ? hardware_cores : std::min(num_threads_request, hardware_cores);
    if (result_obj.threadsUsed == 0) result_obj.threadsUsed = 1;

    int max_orig_dim = std::max({ A_orig.rows(), A_orig.cols(), B_orig.rows(), B_orig.cols() });
    int padded_size = nextPowerOf2(max_orig_dim);

    unsigned long long padded_dim_ull = static_cast<unsigned long long>(padded_size);
    unsigned long long padded_elements = (padded_dim_ull > 0 && padded_dim_ull > std::numeric_limits<unsigned long long>::max() / padded_dim_ull)
        ? std::numeric_limits<unsigned long long>::max() : padded_dim_ull * padded_dim_ull;

    if (padded_elements > std::vector<double>().max_size() || padded_elements == std::numeric_limits<unsigned long long>::max()) {
        throw std::bad_alloc();
    }

    auto total_op_start_chrono = std::chrono::high_resolution_clock::now();
    LARGE_INTEGER total_op_start_qpc = { 0 };
    if (g_performanceFrequency.QuadPart != 0) QueryPerformanceCounter(&total_op_start_qpc);

    auto pad_start = std::chrono::high_resolution_clock::now();
    Matrix Apad = Matrix::pad(A_orig, padded_size);
    Matrix Bpad = Matrix::pad(B_orig, padded_size);
    auto pad_end = std::chrono::high_resolution_clock::now();
    result_obj.padding_duration_sec = std::chrono::duration<double>(pad_end - pad_start).count();

    Matrix Cpad(padded_size, padded_size);

    int max_depth_async = 0;
    if (result_obj.threadsUsed > 1) {
        max_depth_async = static_cast<int>(std::floor(std::log(static_cast<double>(result_obj.threadsUsed)) / std::log(7.0)));
        if (max_depth_async < 0) max_depth_async = 0;
    }

    // --- NEW: Progress Bar Variables ---
    std::atomic<int> progress_counter(0);
    std::atomic<bool> multiplication_done(false);
    std::thread progress_thread;
    bool progress_bar_active = false;
    long long total_tasks = 0;
    int effective_threshold = (threshold <= 0) ? 1 : threshold; // Use 1 if 0 for calculation

    // --- Check if Strassen will be used (and thus progress bar) ---
    if (padded_size > threshold && threshold > 0) { // Only run Strassen if > threshold AND threshold is positive
        result_obj.strassen_applied_at_top_level = true;
        total_tasks = calculate_total_tasks(padded_size, threshold);
        // Only launch progress bar if total tasks > 1 (i.e., not a single naive call)
        if (total_tasks > 1) {
            print_line_in_box(CYAN + " Starting parallel Strassen (Progress bar active)..." + RESET, 80, false);
            progress_bar_active = true;
            progress_thread = std::thread(display_progress, std::ref(progress_counter), total_tasks, std::ref(multiplication_done));
        }
        else {
            print_line_in_box(CYAN + " Starting Strassen (single task)..." + RESET, 80, false);
            result_obj.strassen_applied_at_top_level = false; // It *is* strassen, but will hit base case immediately. Treat as naive.
        }
    }
    else {
        result_obj.strassen_applied_at_top_level = false;
        print_line_in_box(CYAN + " Using Naive multiplication (Size <= Threshold or Threshold=0)..." + RESET, 80, false);
    }


    // --- Perform Multiplication ---
    if (result_obj.strassen_applied_at_top_level) {
        int n2 = padded_size / 2;
        Matrix A11, A12, A21, A22, B11, B12, B21, B22;

        auto split_start = std::chrono::high_resolution_clock::now();
        Apad.split(A11, A12, A21, A22);
        Bpad.split(B11, B12, B21, B22);
        auto split_end = std::chrono::high_resolution_clock::now();
        result_obj.first_level_split_sec = std::chrono::duration<double>(split_end - split_start).count();

        auto S_calc_start = std::chrono::high_resolution_clock::now();
        Matrix S1 = B12 - B22; Matrix S2 = A11 + A12; Matrix S3 = A21 + A22;
        Matrix S4 = B21 - B11; Matrix S5 = A11 + A22; Matrix S6 = B11 + B22;
        Matrix S7 = A12 - A22; Matrix S8 = B21 + B22; Matrix S9 = A21 - A11;
        Matrix S10 = B11 + B12;
        auto S_calc_end = std::chrono::high_resolution_clock::now();
        result_obj.first_level_S_calc_sec = std::chrono::duration<double>(S_calc_end - S_calc_start).count();

        auto P_tasks_start = std::chrono::high_resolution_clock::now();
        std::future<Matrix> fP1 = std::async(std::launch::async, strassen_recursive_worker, std::move(S5), std::move(S6), threshold, 1, max_depth_async, std::ref(progress_counter));
        std::future<Matrix> fP2 = std::async(std::launch::async, strassen_recursive_worker, std::move(S3), std::move(B11), threshold, 1, max_depth_async, std::ref(progress_counter));
        std::future<Matrix> fP3 = std::async(std::launch::async, strassen_recursive_worker, std::move(A11), std::move(S1), threshold, 1, max_depth_async, std::ref(progress_counter));
        std::future<Matrix> fP4 = std::async(std::launch::async, strassen_recursive_worker, std::move(A22), std::move(S4), threshold, 1, max_depth_async, std::ref(progress_counter));
        std::future<Matrix> fP5 = std::async(std::launch::async, strassen_recursive_worker, std::move(S2), std::move(B22), threshold, 1, max_depth_async, std::ref(progress_counter));
        std::future<Matrix> fP6 = std::async(std::launch::async, strassen_recursive_worker, std::move(S9), std::move(S10), threshold, 1, max_depth_async, std::ref(progress_counter));
        std::future<Matrix> fP7 = std::async(std::launch::async, strassen_recursive_worker, std::move(S7), std::move(S8), threshold, 1, max_depth_async, std::ref(progress_counter));

        Matrix P1 = fP1.get(); Matrix P2 = fP2.get(); Matrix P3 = fP3.get();
        Matrix P4 = fP4.get(); Matrix P5 = fP5.get(); Matrix P6 = fP6.get();
        Matrix P7 = fP7.get();
        auto P_tasks_end = std::chrono::high_resolution_clock::now();
        result_obj.first_level_P_tasks_wall_sec = std::chrono::duration<double>(P_tasks_end - P_tasks_start).count();

        auto C_quad_calc_start = std::chrono::high_resolution_clock::now();
        Matrix C11 = P1 + P4 - P5 + P7; Matrix C12 = P3 + P5;
        Matrix C21 = P2 + P4;           Matrix C22 = P1 - P2 + P3 + P6;
        auto C_quad_calc_end = std::chrono::high_resolution_clock::now();
        result_obj.first_level_C_quad_calc_sec = std::chrono::duration<double>(C_quad_calc_end - C_quad_calc_start).count();

        auto final_combine_start = std::chrono::high_resolution_clock::now();
        Cpad = Matrix::combine(C11, C12, C21, C22);
        auto final_combine_end = std::chrono::high_resolution_clock::now();
        result_obj.first_level_final_combine_sec = std::chrono::duration<double>(final_combine_end - final_combine_start).count();
    }
    else { // Use Naive (either because size <= threshold or threshold = 0)
        Cpad = Apad.multiply_naive(Bpad);
    }

    // --- NEW: Stop and Join Progress Bar Thread ---
    if (progress_bar_active) {
        multiplication_done.store(true, std::memory_order_release);
        if (progress_thread.joinable()) {
            progress_thread.join();
        }
    }

    auto unpad_start = std::chrono::high_resolution_clock::now();
    result_obj.resultMatrix = Matrix::unpad(Cpad, A_orig.rows(), B_orig.cols());
    auto unpad_end = std::chrono::high_resolution_clock::now();
    result_obj.unpadding_duration_sec = std::chrono::duration<double>(unpad_end - unpad_start).count();

    auto total_op_end_chrono = std::chrono::high_resolution_clock::now();
    LARGE_INTEGER total_op_end_qpc = { 0 };
    if (g_performanceFrequency.QuadPart != 0) QueryPerformanceCounter(&total_op_end_qpc);

    result_obj.durationSeconds_chrono = std::chrono::duration<double>(total_op_end_chrono - total_op_start_chrono).count();
    result_obj.durationNanoseconds_chrono = std::chrono::duration_cast<std::chrono::nanoseconds>(total_op_end_chrono - total_op_start_chrono).count();
    if (g_performanceFrequency.QuadPart != 0 && g_performanceFrequency.QuadPart > 0) {
        result_obj.durationSeconds_qpc = static_cast<double>(total_op_end_qpc.QuadPart - total_op_start_qpc.QuadPart) / g_performanceFrequency.QuadPart;
    }
    result_obj.memoryInfo = getProcessMemoryUsage();
    return result_obj;
}

// Updated Worker Function (Takes Matrix by value, adds atomic ref)
Matrix strassen_recursive_worker(Matrix A, Matrix B, int threshold, int current_depth, int max_depth_async, std::atomic<int>& progress_counter) {
    // Base case: threshold or empty. Threshold=0 means it only stops at 0x0.
    if (A.rows() <= threshold || A.rows() == 0) {
        if (A.isEmpty() || B.isEmpty()) return Matrix(A.rows(), B.cols());
        Matrix res = A.multiply_naive(B);
        // Increment the counter ONLY when a base task is *actually* computed.
        progress_counter.fetch_add(1, std::memory_order_relaxed);
        return res;
    }
    if (A.rows() % 2 != 0) {
        throw std::logic_error("Internal Strassen: Non-even dimension matrix in recursion where not expected.");
    }

    int n2 = A.rows() / 2;
    Matrix A11, A12, A21, A22, B11, B12, B21, B22;
    // Split needs A and B, but they are now non-const. We need a way to split
    // *or* change split to take non-const. Let's make split take const& as it doesn't modify.
    // The original code passed A_rec.split - this was a member function. It should work.
    // Ah, I changed the signature to `Matrix A, Matrix B`. So A and B are local copies/moves.
    // We *can* split them.
    A.split(A11, A12, A21, A22);
    B.split(B11, B12, B21, B22);

    Matrix S1 = B12 - B22; Matrix S2 = A11 + A12; Matrix S3 = A21 + A22;
    Matrix S4 = B21 - B11; Matrix S5 = A11 + A22; Matrix S6 = B11 + B22;
    Matrix S7 = A12 - A22; Matrix S8 = B21 + B22; Matrix S9 = A21 - A11;
    Matrix S10 = B11 + B12;

    Matrix P1, P2, P3, P4, P5, P6, P7;
    bool launch_async_here = (current_depth <= max_depth_async);

    if (launch_async_here) {
        std::future<Matrix> fP1 = std::async(std::launch::async, strassen_recursive_worker, std::move(S5), std::move(S6), threshold, current_depth + 1, max_depth_async, std::ref(progress_counter));
        std::future<Matrix> fP2 = std::async(std::launch::async, strassen_recursive_worker, std::move(S3), std::move(B11), threshold, current_depth + 1, max_depth_async, std::ref(progress_counter));
        std::future<Matrix> fP3 = std::async(std::launch::async, strassen_recursive_worker, std::move(A11), std::move(S1), threshold, current_depth + 1, max_depth_async, std::ref(progress_counter));
        std::future<Matrix> fP4 = std::async(std::launch::async, strassen_recursive_worker, std::move(A22), std::move(S4), threshold, current_depth + 1, max_depth_async, std::ref(progress_counter));
        std::future<Matrix> fP5 = std::async(std::launch::async, strassen_recursive_worker, std::move(S2), std::move(B22), threshold, current_depth + 1, max_depth_async, std::ref(progress_counter));
        std::future<Matrix> fP6 = std::async(std::launch::async, strassen_recursive_worker, std::move(S9), std::move(S10), threshold, current_depth + 1, max_depth_async, std::ref(progress_counter));
        std::future<Matrix> fP7 = std::async(std::launch::async, strassen_recursive_worker, std::move(S7), std::move(S8), threshold, current_depth + 1, max_depth_async, std::ref(progress_counter));
        P1 = fP1.get(); P2 = fP2.get(); P3 = fP3.get(); P4 = fP4.get(); P5 = fP5.get(); P6 = fP6.get(); P7 = fP7.get();
    }
    else {
        P1 = strassen_recursive_worker(std::move(S5), std::move(S6), threshold, current_depth + 1, max_depth_async, progress_counter);
        P2 = strassen_recursive_worker(std::move(S3), std::move(B11), threshold, current_depth + 1, max_depth_async, progress_counter);
        P3 = strassen_recursive_worker(std::move(A11), std::move(S1), threshold, current_depth + 1, max_depth_async, progress_counter);
        P4 = strassen_recursive_worker(std::move(A22), std::move(S4), threshold, current_depth + 1, max_depth_async, progress_counter);
        P5 = strassen_recursive_worker(std::move(S2), std::move(B22), threshold, current_depth + 1, max_depth_async, progress_counter);
        P6 = strassen_recursive_worker(std::move(S9), std::move(S10), threshold, current_depth + 1, max_depth_async, progress_counter);
        P7 = strassen_recursive_worker(std::move(S7), std::move(S8), threshold, current_depth + 1, max_depth_async, progress_counter);
    }

    Matrix C11 = P1 + P4 - P5 + P7; Matrix C12 = P3 + P5;
    Matrix C21 = P2 + P4;           Matrix C22 = P1 - P2 + P3 + P6;

    return Matrix::combine(C11, C12, C21, C22);
}


// --- Parallel Matrix Comparison Implementation ---
struct ComparisonResult {
    long long matchCount;
    double durationSeconds_chrono;
    long long durationNanoseconds_chrono;
    double durationSeconds_qpc;
    unsigned int threadsUsed;
    unsigned int coresDetected;
    ProcessMemoryInfo memoryInfo;
    int comparisonThreshold;
    double epsilon;
    int originalRows, originalCols;

    ComparisonResult() :
        matchCount(0LL), durationSeconds_chrono(0.0), durationNanoseconds_chrono(0LL),
        durationSeconds_qpc(0.0), threadsUsed(0), coresDetected(0),
        memoryInfo({ 0 }), comparisonThreshold(0), epsilon(0.0),
        originalRows(0), originalCols(0) {
    }
};

long long compareMatricesInternal(const Matrix& A, const Matrix& B, int threshold, double epsilon, int current_depth, int max_depth_async_comp); // Fwd Decl.

ComparisonResult compareMatricesParallel(const Matrix& A_orig, const Matrix& B_orig, int threshold, double epsilon, unsigned int num_threads_request = 0) {
    ComparisonResult result_obj;
    result_obj.originalRows = A_orig.rows();
    result_obj.originalCols = A_orig.cols();
    result_obj.comparisonThreshold = threshold;
    result_obj.epsilon = epsilon;

    if (A_orig.rows() != B_orig.rows() || A_orig.cols() != B_orig.cols()) {
        throw std::invalid_argument("Matrix dimensions must be identical for comparison.");
    }
    if (A_orig.isEmpty()) {
        result_obj.memoryInfo = getProcessMemoryUsage();
        result_obj.coresDetected = getCpuCoreCount();
        result_obj.matchCount = 0;
        return result_obj;
    }

    int max_orig_dim = std::max(A_orig.rows(), A_orig.cols());
    int padded_size = nextPowerOf2(max_orig_dim);

    unsigned long long padded_dim_ull = static_cast<unsigned long long>(padded_size);
    unsigned long long padded_elements = (padded_dim_ull > 0 && padded_dim_ull > std::numeric_limits<unsigned long long>::max() / padded_dim_ull)
        ? std::numeric_limits<unsigned long long>::max() : padded_dim_ull * padded_dim_ull;

    if (padded_elements > std::vector<double>().max_size() || padded_elements == std::numeric_limits<unsigned long long>::max()) {
        throw std::bad_alloc();
    }

    const Matrix Apad = Matrix::pad(A_orig, padded_size);
    const Matrix Bpad = Matrix::pad(B_orig, padded_size);

    unsigned int hardware_cores = getCpuCoreCount();
    result_obj.coresDetected = hardware_cores;
    result_obj.threadsUsed = (num_threads_request == 0) ? hardware_cores : std::min(num_threads_request, hardware_cores);
    if (result_obj.threadsUsed == 0) result_obj.threadsUsed = 1;

    int max_depth_async_comp = 0;
    if (result_obj.threadsUsed > 1) {
        max_depth_async_comp = static_cast<int>(std::floor(std::log(static_cast<double>(result_obj.threadsUsed)) / std::log(4.0)));
        if (max_depth_async_comp < 0) max_depth_async_comp = 0;
    }

    auto start_time_chrono = std::chrono::high_resolution_clock::now();
    LARGE_INTEGER start_time_qpc = { 0 };
    if (g_performanceFrequency.QuadPart != 0) QueryPerformanceCounter(&start_time_qpc);

    result_obj.matchCount = compareMatricesInternal(Apad, Bpad, threshold, epsilon, 0, max_depth_async_comp);

    auto end_time_chrono = std::chrono::high_resolution_clock::now();
    LARGE_INTEGER end_time_qpc = { 0 };
    if (g_performanceFrequency.QuadPart != 0) QueryPerformanceCounter(&end_time_qpc);

    result_obj.durationSeconds_chrono = std::chrono::duration<double>(end_time_chrono - start_time_chrono).count();
    result_obj.durationNanoseconds_chrono = std::chrono::duration_cast<std::chrono::nanoseconds>(end_time_chrono - start_time_chrono).count();
    if (g_performanceFrequency.QuadPart != 0 && g_performanceFrequency.QuadPart > 0) {
        result_obj.durationSeconds_qpc = static_cast<double>(end_time_qpc.QuadPart - start_time_qpc.QuadPart) / g_performanceFrequency.QuadPart;
    }
    result_obj.memoryInfo = getProcessMemoryUsage();
    return result_obj;
}

long long compareMatricesInternal(const Matrix& A_rec, const Matrix& B_rec, int threshold, double epsilon, int current_depth, int max_depth_async_comp) {
    if (A_rec.rows() <= threshold || A_rec.rows() == 0) {
        return A_rec.compare_naive(B_rec, epsilon);
    }
    if (A_rec.rows() % 2 != 0) {
        throw std::logic_error("Internal Compare: Non-even dimension matrix in recursion.");
    }

    int n2 = A_rec.rows() / 2;
    Matrix A11, A12, A21, A22, B11, B12, B21, B22;
    Matrix::split(A_rec, B_rec, A11, A12, A21, A22, B11, B12, B21, B22);

    bool launch_async_here = (current_depth <= max_depth_async_comp);

    if (launch_async_here) {
        std::future<long long> f_c11 = std::async(std::launch::async, compareMatricesInternal, std::move(A11), std::move(B11), threshold, epsilon, current_depth + 1, max_depth_async_comp);
        std::future<long long> f_c12 = std::async(std::launch::async, compareMatricesInternal, std::move(A12), std::move(B12), threshold, epsilon, current_depth + 1, max_depth_async_comp);
        std::future<long long> f_c21 = std::async(std::launch::async, compareMatricesInternal, std::move(A21), std::move(B21), threshold, epsilon, current_depth + 1, max_depth_async_comp);
        std::future<long long> f_c22 = std::async(std::launch::async, compareMatricesInternal, std::move(A22), std::move(B22), threshold, epsilon, current_depth + 1, max_depth_async_comp);
        return f_c11.get() + f_c12.get() + f_c21.get() + f_c22.get();
    }
    else {
        return compareMatricesInternal(A11, B11, threshold, epsilon, current_depth + 1, max_depth_async_comp) +
            compareMatricesInternal(A12, B12, threshold, epsilon, current_depth + 1, max_depth_async_comp) +
            compareMatricesInternal(A21, B21, threshold, epsilon, current_depth + 1, max_depth_async_comp) +
            compareMatricesInternal(A22, B22, threshold, epsilon, current_depth + 1, max_depth_async_comp);
    }
}

// --- CSV Logging Functions ---
void logMultiplicationResultToCSV(const MultiplicationResult& result, const std::string& filename) {
    std::ofstream logfile(filename, std::ios::out | std::ios::app);
    if (!logfile.is_open()) {
        cerr << RED << "Error: Could not open log file: " << filename << RESET << endl; return;
    }

    logfile.seekp(0, std::ios::end);
    bool is_empty = (logfile.tellp() == 0);

    if (is_empty) {
        logfile << "Operation,RowsA,ColsA,RowsB,ColsB,ResultRows,ResultCols,TotalElementsResult,"
            << "DurationSeconds_Chrono,DurationNanoseconds_Chrono,DurationSeconds_QPC,"
            << "ThreadsUsed,CoresDetected,PeakMemoryMB,StrassenThreshold,"
            << "StrassenAppliedTopLevel,Padding_sec,Unpadding_sec,"
            << "Split_L1_sec,S_Calc_L1_sec,P_Tasks_L1_Wall_sec,C_Quad_Calc_L1_sec,Final_Combine_L1_sec\n";
    }

    std::ios_base::fmtflags original_flags = logfile.flags();
    std::streamsize original_precision = logfile.precision();
    logfile << std::fixed << std::setprecision(10);

    logfile << "Multiplication,"
        << result.originalRowsA << "," << result.originalColsA << ","
        << result.originalRowsB << "," << result.originalColsB << ","
        << result.resultMatrix.rows() << "," << result.resultMatrix.cols() << ","
        << result.resultMatrix.elementCount() << ","
        << result.durationSeconds_chrono << "," << result.durationNanoseconds_chrono << ","
        << result.durationSeconds_qpc << "," << result.threadsUsed << ","
        << result.coresDetected << "," << result.memoryInfo.peakWorkingSetMB << ","
        << result.strassenThreshold << ","
        << (result.strassen_applied_at_top_level ? "Yes" : "No") << ","
        << result.padding_duration_sec << "," << result.unpadding_duration_sec << ",";

    if (result.strassen_applied_at_top_level) {
        logfile << result.first_level_split_sec << "," << result.first_level_S_calc_sec << ","
            << result.first_level_P_tasks_wall_sec << "," << result.first_level_C_quad_calc_sec << ","
            << result.first_level_final_combine_sec;
    }
    else {
        logfile << "0.0,0.0,0.0,0.0,0.0";
    }
    logfile << "\n";

    logfile.flags(original_flags); logfile.precision(original_precision);
    logfile.close();
    if (logfile.fail()) cerr << RED << "Warning: Error occurred while writing to log file: " << filename << RESET << endl;
    else cout << GREEN << "Multiplication result logged to " << filename << RESET << endl;
}

void logComparisonResultToCSV(const ComparisonResult& result, const std::string& filename) {
    std::ofstream logfile(filename, std::ios::out | std::ios::app);
    if (!logfile.is_open()) {
        cerr << RED << "Error: Could not open log file: " << filename << RESET << endl; return;
    }
    logfile.seekp(0, std::ios::end);
    bool is_empty = (logfile.tellp() == 0);

    if (is_empty) {
        logfile << "Operation,Rows,Cols,TotalElements,MatchCount,MismatchCount,MatchPercentage,"
            << "DurationSeconds_Chrono,DurationNanoseconds_Chrono,DurationSeconds_QPC,"
            << "ThreadsUsed,CoresDetected,PeakMemoryMB,ComparisonThreshold,Epsilon\n";
    }

    std::ios_base::fmtflags original_flags = logfile.flags();
    std::streamsize original_precision = logfile.precision();
    logfile << std::fixed << std::setprecision(10);

    long long total_elements = static_cast<long long>(result.originalRows) * result.originalCols;
    double match_percentage = (total_elements > 0) ? (static_cast<double>(result.matchCount) / total_elements) * 100.0 : 0.0;

    logfile << "Comparison,"
        << result.originalRows << "," << result.originalCols << "," << total_elements << ","
        << result.matchCount << "," << (total_elements - result.matchCount) << ","
        << std::fixed << std::setprecision(2) << match_percentage << ","
        << std::fixed << std::setprecision(10) << result.durationSeconds_chrono << ","
        << result.durationNanoseconds_chrono << "," << result.durationSeconds_qpc << ","
        << result.threadsUsed << "," << result.coresDetected << ","
        << result.memoryInfo.peakWorkingSetMB << "," << result.comparisonThreshold << ","
        << std::scientific << std::setprecision(10) << result.epsilon << "\n";

    logfile.flags(original_flags); logfile.precision(original_precision);
    logfile.close();
    if (logfile.fail()) cerr << RED << "Warning: Error occurred while writing to log file: " << filename << RESET << endl;
    else cout << GREEN << "Comparison result logged to " << filename << RESET << endl;
}


// --- Main Function Helpers ---
// #define PRINT_MATRICES // Uncomment to print matrices for debugging

void clear_input_buffer_after_cin() {
    cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}

template<typename T>
T get_valid_input(const std::string& prompt_text_before_input_field) {
    T value;
    while (true) {
        cout << prompt_text_before_input_field << YELLOW;
        cin >> value;
        cout << RESET;
        if (cin.good()) {
            if (!std::is_same<T, std::string>::value) {
                char next_char = static_cast<char>(cin.peek());
                if (next_char != '\n' && next_char != EOF && !isspace(next_char)) {
                    cerr << RED << "\nInvalid input: Extra characters after the number ('" << next_char << "...'). Please enter only the number." << RESET << endl;
                    cin.clear();
                    clear_input_buffer_after_cin();
                    continue;
                }
            }
            clear_input_buffer_after_cin();
            return value;
        }
        else {
            cerr << RED << "\nInvalid input format. Please try again." << RESET << endl;
            cin.clear();
            clear_input_buffer_after_cin();
        }
    }
}

template<>
string get_valid_input<string>(const std::string& prompt_text_before_input_field) {
    string value;
    while (true) {
        cout << prompt_text_before_input_field << YELLOW;
        std::getline(cin >> std::ws, value);
        cout << RESET;
        if (!value.empty()) {
            return value;
        }
        else {
            cerr << RED << "\nInput cannot be empty. Please try again." << RESET << endl;
        }
    }
}

void display_detailed_timings_ascii_chart(const MultiplicationResult& result) {
    if (!result.strassen_applied_at_top_level &&
        (result.padding_duration_sec == 0 && result.unpadding_duration_sec == 0 && result.durationSeconds_chrono < 0.0001)) {
        return;
    }

    print_header_box("Detailed Step Timings (ASCII Chart)", 80);

    struct TimingEntry { string label; double time_sec; };
    std::vector<TimingEntry> timings;
    double total_timed_sec = 0;

    if (result.strassen_applied_at_top_level) {
        timings.push_back({ "Padding", result.padding_duration_sec });
        timings.push_back({ "Split L1", result.first_level_split_sec });
        timings.push_back({ "S-Matrices L1", result.first_level_S_calc_sec });
        timings.push_back({ "P-Tasks L1 (Wall)", result.first_level_P_tasks_wall_sec });
        timings.push_back({ "C-Quads L1", result.first_level_C_quad_calc_sec });
        timings.push_back({ "Combine L1", result.first_level_final_combine_sec });
        timings.push_back({ "Unpadding", result.unpadding_duration_sec });
        for (const auto& t : timings) total_timed_sec += t.time_sec;
    }
    else {
        timings.push_back({ "Padding", result.padding_duration_sec });
        double naive_compute_sec = result.durationSeconds_chrono - result.padding_duration_sec - result.unpadding_duration_sec;
        timings.push_back({ "Main Compute", std::max(0.0, naive_compute_sec) });
        timings.push_back({ "Unpadding", result.unpadding_duration_sec });
        for (const auto& t : timings) total_timed_sec += t.time_sec;
    }

    if (std::abs(total_timed_sec - result.durationSeconds_chrono) > 0.001 && result.durationSeconds_chrono > total_timed_sec) {
        timings.push_back({ "Other/Overhead", result.durationSeconds_chrono - total_timed_sec });
    }

    double max_time_entry = 0.0;
    for (const auto& entry : timings) {
        if (entry.time_sec > max_time_entry) max_time_entry = entry.time_sec;
    }

    const int chart_width = 35;
    int label_width = 20;

    for (const auto& entry : timings) {
        std::stringstream line_ss;
        line_ss << std::left << std::setw(label_width) << (entry.label + ":") << " ";
        int bar_length = (max_time_entry > 1e-9) ? static_cast<int>((entry.time_sec / max_time_entry) * chart_width) : 0;
        bar_length = std::min(chart_width, std::max(0, bar_length));

        line_ss << GREEN << std::string(bar_length, '#') << RESET;
        line_ss << std::string(chart_width - bar_length, ' ');
        line_ss << " (" << std::fixed << std::setprecision(4) << entry.time_sec << "s)";
        print_line_in_box(line_ss.str(), 80, false);
    }
    if (result.strassen_applied_at_top_level)
        print_line_in_box(DARK_GRAY + "P-Tasks L1 (Wall) = total time for parallel sub-problems." + RESET, 80, false);
    print_footer_box(80); cout << endl;
}

// --- NEW: Function to run one full operation (Multiply or Compare) ---
void run_one_operation() {
    SystemMemoryInfo sysMemInfo = getSystemMemoryInfo();
    unsigned int coreCount = getCpuCoreCount();
    std::stringstream ss_line;
    int info_label_width = 25;

    print_header_box("System Information", 80);
    ss_line.str(""); ss_line << std::left << std::setw(info_label_width) << " Total Physical RAM :" << PURPLE << sysMemInfo.totalPhysicalMB << " MB" << RESET; print_line_in_box(ss_line.str(), 80, false);
    ss_line.str(""); ss_line << std::left << std::setw(info_label_width) << " Available Physical RAM :" << GREEN << sysMemInfo.availablePhysicalMB << " MB" << RESET; print_line_in_box(ss_line.str(), 80, false);
    ss_line.str(""); ss_line << std::left << std::setw(info_label_width) << " Logical CPU Cores :" << BLUE << coreCount << RESET; print_line_in_box(ss_line.str(), 80, false);
    check_simd_support();
    ss_line.str(""); ss_line << std::left << std::setw(info_label_width) << " SIMD Support :";
    if (has_avx) ss_line << GREEN << "AVX Enabled" << RESET;
    else if (has_sse2) ss_line << YELLOW << "SSE2 Enabled (AVX Not Optimal/Found)" << RESET;
    else ss_line << RED << "Scalar (No AVX/SSE2)" << RESET;
    print_line_in_box(ss_line.str(), 80, false);
    print_footer_box(80); cout << endl;

    print_header_box("Select Operation", 80);
    print_line_in_box(" 1. Matrix Multiplication (Strassen Parallel)", 80);
    print_line_in_box(" 2. Matrix Comparison (Recursive Parallel)", 80);
    print_footer_box(80);
    int operation_choice = get_valid_input<int>(" Enter choice (1 or 2): "); cout << endl;

    print_header_box("Performance Logging", 80); print_footer_box(80);
    char log_choice = get_valid_input<char>(" Log results to CSV? (y/n): "); cout << endl;
    string log_filename = "";
    if (tolower(log_choice) == 'y') {
        print_header_box("Log File Name", 80); print_footer_box(80);
        log_filename = get_valid_input<string>(" Enter log filename (e.g., perf_log.csv): "); cout << endl;
    }
    cout << endl;

    if (operation_choice == 1) { // ---- MULTIPLICATION ----
        print_header_box("Matrix Dimensions (Multiplication)", 80); print_footer_box(80);
        int rowsA = get_valid_input<int>(" Matrix A - Rows: ");
        int colsA = get_valid_input<int>(" Matrix A - Cols: ");
        int rowsB = get_valid_input<int>(" Matrix B - Rows: ");
        int colsB = get_valid_input<int>(" Matrix B - Cols: "); cout << endl;

        if (rowsA < 0 || colsA < 0 || rowsB < 0 || colsB < 0) { throw std::invalid_argument("Matrix dimensions cannot be negative."); }
        if (colsA != rowsB) { throw std::invalid_argument("Incompatible dimensions (A.cols != B.rows)."); }

        print_header_box("Memory Estimation (Multiplication)", 80);
        int max_orig_dim_mult = std::max({ rowsA, colsA, rowsB, colsB });
        int padded_n_mult = nextPowerOf2(max_orig_dim_mult);
        unsigned long long estimatedMB_mult = estimateStrassenMemoryMB(padded_n_mult);
        ss_line.str(""); ss_line << BLUE << " Est. peak RAM (Strassen): ~" << estimatedMB_mult << " MB " << RESET
            << DARK_GRAY << "(for " << padded_n_mult << "x" << padded_n_mult << " padded)" << RESET; print_line_in_box(ss_line.str(), 80, false);
        if (sysMemInfo.availablePhysicalMB > 0 && estimatedMB_mult > sysMemInfo.availablePhysicalMB * 0.75) {
            print_line_in_box(RED + " Warning: Estimated RAM is high vs. available. Risk of slow/fail." + RESET, 80, false);
        }
        else { print_line_in_box(GREEN + " Estimated RAM seems acceptable vs. available." + RESET, 80, false); }
        print_footer_box(80); cout << endl;
        if (rowsA == 0 || colsA == 0 || rowsB == 0 || colsB == 0) {
            cout << YELLOW << "Input results in an empty or trivial multiplication." << RESET << endl;
        }

        print_header_box("Matrix Input Method", 80);
        print_line_in_box(" 1. Random Generation ", 80);
        print_line_in_box(" 2. Manual Console Input (small matrices)", 80);
        print_line_in_box(" 3. Read from File", 80); print_footer_box(80);
        int input_choice_mult = get_valid_input<int>(" Enter choice (1-3): "); cout << endl;

        Matrix A(rowsA, colsA), B(rowsB, colsB);
        auto gen_start_time_mult = std::chrono::high_resolution_clock::now();
        int gen_spinner_idx_mult = 0;

        switch (input_choice_mult) {
        case 1: {
            cout << CYAN << "Generating Matrix A (" << rowsA << "x" << colsA << ") and B (" << rowsB << "x" << colsB << ")..." << RESET;
            std::future<Matrix> futureA = std::async(std::launch::async, &Matrix::generateRandom, rowsA, colsA);
            std::future<Matrix> futureB = std::async(std::launch::async, &Matrix::generateRandom, rowsB, colsB);
            while (futureA.wait_for(std::chrono::milliseconds(0)) != std::future_status::ready ||
                futureB.wait_for(std::chrono::milliseconds(0)) != std::future_status::ready) {
                cout << "\b" << SPINNER_CHARS[gen_spinner_idx_mult] << std::flush;
                gen_spinner_idx_mult = (gen_spinner_idx_mult + 1) % NUM_SPINNER_CHARS;
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }
            A = futureA.get(); B = futureB.get();
            cout << "\b " << GREEN << "Done." << RESET << endl;
            break;
        }
        case 2: {
            int dummy_spinner = 0;
            if (rowsA * colsA > 100 || rowsB * colsB > 100) cout << YELLOW << "Warning: Manual input for large matrices is not recommended." << RESET << endl;
            A = Matrix::readFromConsole(rowsA, colsA, dummy_spinner);
            B = Matrix::readFromConsole(rowsB, colsB, dummy_spinner);
            break;
        }
        case 3: {
            string filenameA = get_valid_input<string>(" Filename for Matrix A: ");
            string filenameB = get_valid_input<string>(" Filename for Matrix B: "); cout << endl;
            Matrix tempA = Matrix::readFromFile(filenameA);
            if (tempA.rows() != rowsA || tempA.cols() != colsA) { throw std::runtime_error("Matrix A from file " + filenameA + " has wrong dimensions."); }
            A = std::move(tempA);
            Matrix tempB = Matrix::readFromFile(filenameB);
            if (tempB.rows() != rowsB || tempB.cols() != colsB) { throw std::runtime_error("Matrix B from file " + filenameB + " has wrong dimensions."); }
            B = std::move(tempB);
            break;
        }
        default: throw std::runtime_error("Invalid input choice.");
        }
        auto gen_end_time_mult = std::chrono::high_resolution_clock::now();
        cout << "Matrix input/generation took: " << GREEN << std::fixed << std::setprecision(4) << std::chrono::duration<double>(gen_end_time_mult - gen_start_time_mult).count() << "s." << RESET << endl << endl;

#ifdef PRINT_MATRICES
        A.print(cout, 2, 10); B.print(cout, 2, 10);
#endif
        print_header_box("Multiplication Settings", 80);
        int strassen_threshold = get_valid_input<int>(" Strassen threshold (e.g., 64; >0 for Strassen, 0 for Naive): ");
        if (strassen_threshold < 0) { throw std::invalid_argument("Threshold cannot be negative."); }
        // Hints based on threshold
        int current_padded_n_mult = nextPowerOf2(std::max({ rowsA, colsA, rowsB, colsB }));
        if (strassen_threshold == 0) print_line_in_box(YELLOW + "Hint: Threshold 0 forces Naive multiplication (or Strassen failsafe)." + RESET, 80, false);
        else if (strassen_threshold >= current_padded_n_mult) print_line_in_box(YELLOW + "Hint: Thresh >= padded_N. Naive will be used." + RESET, 80, false);

        unsigned int num_threads_req_mult = get_valid_input<unsigned int>(" Threads to use (0 for auto, max " + std::to_string(coreCount) + "): ");
        print_footer_box(80); cout << endl;


        print_header_box("Performing Multiplication", 80);
        // Message moved inside multiplyStrassenParallel
        MultiplicationResult mult_res = multiplyStrassenParallel(A, B, strassen_threshold, num_threads_req_mult);
        play_completion_sound();
        cout << GREEN << "\n--- Multiplication Complete ---" << RESET << endl << endl;

        print_header_box("Results & Statistics (Multiplication)", 80); int metric_label_width_mult = 28;
#ifdef PRINT_MATRICES
        mult_res.resultMatrix.print(cout, 2, 10);
#else
        print_line_in_box(DARK_GRAY + "(Result matrix preview disabled via PRINT_MATRICES.)" + RESET, 80, false);
#endif
        ss_line.str(""); ss_line << std::left << std::setw(metric_label_width_mult) << " Input A Dimensions :" << YELLOW << mult_res.originalRowsA << "x" << mult_res.originalColsA << RESET; print_line_in_box(ss_line.str(), 80, false);
        ss_line.str(""); ss_line << std::left << std::setw(metric_label_width_mult) << " Input B Dimensions :" << YELLOW << mult_res.originalRowsB << "x" << mult_res.originalColsB << RESET; print_line_in_box(ss_line.str(), 80, false);
        ss_line.str(""); ss_line << std::left << std::setw(metric_label_width_mult) << " Final Matrix Dimensions :" << YELLOW << mult_res.resultMatrix.rows() << "x" << mult_res.resultMatrix.cols() << RESET; print_line_in_box(ss_line.str(), 80, false);
        print_separator_line(80);
        ss_line.str(""); ss_line << std::left << std::setw(metric_label_width_mult) << " Exec. Time (chrono) :" << GREEN << std::fixed << std::setprecision(4) << mult_res.durationSeconds_chrono << " s" << RESET; print_line_in_box(ss_line.str(), 80, false);
        if (g_performanceFrequency.QuadPart > 0) { ss_line.str(""); ss_line << std::left << std::setw(metric_label_width_mult) << " Exec. Time (QPC) :" << GREEN << std::fixed << std::setprecision(6) << mult_res.durationSeconds_qpc << " s" << RESET; print_line_in_box(ss_line.str(), 80, false); }
        ss_line.str(""); ss_line << std::left << std::setw(metric_label_width_mult) << " Threads Used :" << CYAN << mult_res.threadsUsed << RESET; print_line_in_box(ss_line.str(), 80, false);
        ss_line.str(""); ss_line << std::left << std::setw(metric_label_width_mult) << " Peak Memory Usage :" << GREEN << mult_res.memoryInfo.peakWorkingSetMB << " MB" << RESET; print_line_in_box(ss_line.str(), 80, false);
        ss_line.str(""); ss_line << std::left << std::setw(metric_label_width_mult) << " Strassen Threshold Used :" << YELLOW << mult_res.strassenThreshold << RESET; print_line_in_box(ss_line.str(), 80, false);
        ss_line.str(""); ss_line << std::left << std::setw(metric_label_width_mult) << " Strassen Top Level Run :" << (mult_res.strassen_applied_at_top_level ? GREEN + "Yes" + RESET : YELLOW + "No" + RESET); print_line_in_box(ss_line.str(), 80, false);
        print_footer_box(80); cout << endl;

        if (!log_filename.empty()) logMultiplicationResultToCSV(mult_res, log_filename);
        display_detailed_timings_ascii_chart(mult_res);

        print_header_box("Save Result Matrix", 80); print_footer_box(80);
        char save_choice_mult = get_valid_input<char>(" Save result matrix C to file? (y/n): ");
        if (tolower(save_choice_mult) == 'y') {
            string save_filename_mult = get_valid_input<string>(" Enter filename for result (e.g., result_C.txt): ");
            mult_res.resultMatrix.saveToFile(save_filename_mult);
        }

    }
    else if (operation_choice == 2) { // ---- COMPARISON ----
        print_header_box("Matrix Dimensions (Comparison)", 80); print_footer_box(80);
        int rowsA_comp = get_valid_input<int>(" Matrix 1 - Rows: "); int colsA_comp = get_valid_input<int>(" Matrix 1 - Cols: ");
        int rowsB_comp = get_valid_input<int>(" Matrix 2 - Rows: "); int colsB_comp = get_valid_input<int>(" Matrix 2 - Cols: "); cout << endl;

        if (rowsA_comp < 0 || colsA_comp < 0 || rowsB_comp < 0 || colsB_comp < 0) { throw std::invalid_argument("Dims cannot be negative."); }
        if (rowsA_comp != rowsB_comp || colsA_comp != colsB_comp) { throw std::invalid_argument("Dims must be identical for comparison."); }

        print_header_box("Memory Estimation (Comparison)", 80);
        int max_orig_dim_comp = std::max(rowsA_comp, colsA_comp);
        int padded_n_comp = nextPowerOf2(max_orig_dim_comp);
        unsigned long long estimatedMB_comp = estimateComparisonMemoryMB(padded_n_comp);
        ss_line.str(""); ss_line << BLUE << " Est. peak RAM: ~" << estimatedMB_comp << " MB " << RESET << DARK_GRAY << "(for " << padded_n_comp << "x" << padded_n_comp << " padded)" << RESET; print_line_in_box(ss_line.str(), 80, false);
        if (sysMemInfo.availablePhysicalMB > 0 && estimatedMB_comp > sysMemInfo.availablePhysicalMB * 0.75) {
            print_line_in_box(RED + " Warning: Est. RAM high vs. available." + RESET, 80, false);
        }
        else { print_line_in_box(GREEN + " Est. RAM seems acceptable." + RESET, 80, false); }
        print_footer_box(80); cout << endl;
        if (rowsA_comp == 0 || colsA_comp == 0) cout << YELLOW << "Comparing empty matrices (0 elements)." << RESET << endl;

        print_header_box("Matrix Input (Comparison)", 80);
        print_line_in_box(" 1. Random Generation (Identical Seeds)", 80);
        print_line_in_box(" 2. Read from File", 80); print_footer_box(80);
        int input_choice_comp = get_valid_input<int>(" Enter choice (1-2): "); cout << endl;

        Matrix A_comp, B_comp;
        auto read_start_comp_time = std::chrono::high_resolution_clock::now();

        if (input_choice_comp == 1) {
            cout << CYAN << "Generating 2 identical random matrices (" << rowsA_comp << "x" << colsA_comp << ")..." << RESET;
            // Use same seed for identical matrices
            std::mt19937 gen(12345); // Fixed seed
            constexpr double minVal = -10.0; constexpr double maxVal = 10.0;
            std::uniform_real_distribution<double> distrib(minVal, maxVal);
            A_comp = Matrix(rowsA_comp, colsA_comp);
            B_comp = Matrix(rowsA_comp, colsA_comp);
            for (int i = 0; i < rowsA_comp; ++i) {
                for (int j = 0; j < colsA_comp; ++j) {
                    A_comp(i, j) = B_comp(i, j) = distrib(gen);
                }
            }
            cout << GREEN << "Done." << RESET << endl;
            // Optionally add a few differences
            if (rowsA_comp > 1 && colsA_comp > 1) {
                B_comp(0, 0) += 1e-5; // Small difference
                cout << YELLOW << "Note: Added small difference to B(0,0) for testing." << RESET << endl;
            }
        }
        else if (input_choice_comp == 2) {
            string filenameA_comp = get_valid_input<string>(" Filename for Matrix 1: ");
            string filenameB_comp = get_valid_input<string>(" Filename for Matrix 2: "); cout << endl;
            Matrix tempA = Matrix::readFromFile(filenameA_comp);
            if (tempA.rows() != rowsA_comp || tempA.cols() != colsA_comp) { throw std::runtime_error("Matrix 1 from file has wrong dimensions."); }
            A_comp = std::move(tempA);
            Matrix tempB = Matrix::readFromFile(filenameB_comp);
            if (tempB.rows() != rowsB_comp || tempB.cols() != colsB_comp) { throw std::runtime_error("Matrix 2 from file has wrong dimensions."); }
            B_comp = std::move(tempB);
        }
        else {
            throw std::runtime_error("Invalid input choice.");
        }

        auto read_end_comp_time = std::chrono::high_resolution_clock::now();
        cout << "Matrix input/generation took: " << GREEN << std::fixed << std::setprecision(4) << std::chrono::duration<double>(read_end_comp_time - read_start_comp_time).count() << "s." << RESET << endl << endl;


#ifdef PRINT_MATRICES
        A_comp.print(cout, 2, 10); B_comp.print(cout, 2, 10);
#endif
        print_header_box("Comparison Settings", 80);
        int comparison_threshold = get_valid_input<int>(" Comparison threshold (e.g., 64; 0 for naive): ");
        if (comparison_threshold < 0) { throw std::invalid_argument("Threshold < 0."); }
        double epsilon_comp = get_valid_input<double>(" Epsilon for float compare (e.g., 1e-9; 0 for exact): ");
        if (epsilon_comp < 0) { throw std::invalid_argument("Epsilon < 0."); }
        if (epsilon_comp == 0) print_line_in_box(YELLOW + "Hint: Exact comparison (epsilon=0)." + RESET, 80, false);
        else { ss_line.str(""); ss_line << YELLOW << "Hint: Tolerance comparison (epsilon=" << std::scientific << epsilon_comp << std::fixed << ")." << RESET; print_line_in_box(ss_line.str(), 80, false); }
        unsigned int num_threads_req_comp = get_valid_input<unsigned int>(" Threads to use (0 for auto, max " + std::to_string(coreCount) + "): ");
        print_footer_box(80); cout << endl;


        print_header_box("Performing Comparison", 80); print_line_in_box(CYAN + " Starting parallel matrix comparison..." + RESET, 80, false); print_footer_box(80); cout << endl;

        ComparisonResult comp_res = compareMatricesParallel(A_comp, B_comp, comparison_threshold, epsilon_comp, num_threads_req_comp);
        play_completion_sound();
        cout << GREEN << "\n--- Comparison Complete ---" << RESET << endl << endl;

        print_header_box("Comparison Results & Statistics", 80); int metric_label_width_comp = 30;
        long long total_elements_comp = static_cast<long long>(comp_res.originalRows) * comp_res.originalCols;
        ss_line.str(""); ss_line << std::left << std::setw(metric_label_width_comp) << " Matrix Dimensions :" << YELLOW << comp_res.originalRows << "x" << comp_res.originalCols << RESET; print_line_in_box(ss_line.str(), 80, false);
        ss_line.str(""); ss_line << std::left << std::setw(metric_label_width_comp) << " Total Elements Compared :" << PURPLE << total_elements_comp << RESET; print_line_in_box(ss_line.str(), 80, false);
        print_separator_line(80);
        ss_line.str(""); ss_line << std::left << std::setw(metric_label_width_comp) << " Matching Elements Found :" << GREEN << comp_res.matchCount << RESET; print_line_in_box(ss_line.str(), 80, false);
        ss_line.str(""); ss_line << std::left << std::setw(metric_label_width_comp) << " Mismatching Elements :" << RED << (total_elements_comp - comp_res.matchCount) << RESET; print_line_in_box(ss_line.str(), 80, false);
        if (total_elements_comp > 0) { ss_line.str(""); ss_line << std::left << std::setw(metric_label_width_comp) << " Match Percentage :" << GREEN << std::fixed << std::setprecision(2) << (static_cast<double>(comp_res.matchCount) / total_elements_comp) * 100.0 << " %" << RESET; print_line_in_box(ss_line.str(), 80, false); }
        else { ss_line.str(""); ss_line << std::left << std::setw(metric_label_width_comp) << " Match Percentage :" << YELLOW << "N/A (0 elements)" << RESET; print_line_in_box(ss_line.str(), 80, false); }
        print_separator_line(80);
        ss_line.str(""); ss_line << std::left << std::setw(metric_label_width_comp) << " Exec. Time (chrono) :" << GREEN << std::fixed << std::setprecision(4) << comp_res.durationSeconds_chrono << " s" << RESET; print_line_in_box(ss_line.str(), 80, false);
        if (g_performanceFrequency.QuadPart > 0) { ss_line.str(""); ss_line << std::left << std::setw(metric_label_width_comp) << " Exec. Time (QPC) :" << GREEN << std::fixed << std::setprecision(6) << comp_res.durationSeconds_qpc << " s" << RESET; print_line_in_box(ss_line.str(), 80, false); }
        ss_line.str(""); ss_line << std::left << std::setw(metric_label_width_comp) << " Threads Used :" << CYAN << comp_res.threadsUsed << RESET; print_line_in_box(ss_line.str(), 80, false);
        ss_line.str(""); ss_line << std::left << std::setw(metric_label_width_comp) << " Peak Memory Usage :" << GREEN << comp_res.memoryInfo.peakWorkingSetMB << " MB" << RESET; print_line_in_box(ss_line.str(), 80, false);
        ss_line.str(""); ss_line << std::left << std::setw(metric_label_width_comp) << " Comparison Threshold Used :" << YELLOW << comp_res.comparisonThreshold << RESET; print_line_in_box(ss_line.str(), 80, false);
        ss_line.str(""); ss_line << std::left << std::setw(metric_label_width_comp) << " Floating Point Epsilon :" << YELLOW << std::scientific << std::setprecision(2) << comp_res.epsilon << std::fixed << RESET; print_line_in_box(ss_line.str(), 80, false);
        print_footer_box(80); cout << endl;

        if (!log_filename.empty()) logComparisonResultToCSV(comp_res, log_filename);

    }
    else {
        throw std::runtime_error("Invalid operation choice.");
    }
}


// --- NEW: Main Function with Interactive Loop ---
int main() {
    SetConsoleOutputCP(CP_UTF8);
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hOut != INVALID_HANDLE_VALUE) {
        DWORD dwMode = 0;
        if (GetConsoleMode(hOut, &dwMode)) {
            dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
            SetConsoleMode(hOut, dwMode);
        }
    }

    initializePerformanceCounter();

    int initial_spinner_idx = 0;
    cout << "Initializing Matrix Operations Program ";
    for (int i = 0; i < 8; ++i) {
        cout << "\b" << SPINNER_CHARS[initial_spinner_idx] << std::flush;
        initial_spinner_idx = (initial_spinner_idx + 1) % NUM_SPINNER_CHARS;
        std::this_thread::sleep_for(std::chrono::milliseconds(120));
    }
    cout << "\b " << GREEN << "Ready!" << RESET << endl << endl;

    print_header_box("Matrix Operations Program (17)v2.2", 80);
    print_line_in_box(CYAN + " Strassen Multiplication & Parallel Comparison " + RESET, 80, false, Alignment::Center);
    print_footer_box(80); cout << endl;

    bool continue_running = true;
    while (continue_running) {
        try {
            run_one_operation(); // Run the chosen operation
        }
        catch (const std::bad_alloc& e) {
            cerr << "\n\n" << RED << "*** CRITICAL: Memory Allocation Error ***" << RESET << endl;
            cerr << RED << "Details: " << e.what() << RESET << endl;
            cerr << RED << "The program requested too much memory. Check available RAM and matrix sizes." << RESET << endl;
            // Optionally ask to continue even after memory error, though next run might fail too.
        }
        catch (const std::exception& e) {
            cerr << "\n\n" << RED << "*** CRITICAL: An Exception Occurred ***" << RESET << endl;
            cerr << RED << "Details: " << e.what() << RESET << endl;
        }
        catch (...) {
            cerr << "\n\n" << RED << "*** CRITICAL: An Unknown Error Occurred ***" << RESET << endl;
        }

        cout << endl;
        print_header_box("Continue?", 80);
        char choice = get_valid_input<char>(" Продолжить? (y/n): ");
        print_footer_box(80);
        if (tolower(choice) != 'y') {
            continue_running = false;
        }
        cout << endl << string(80, '=') << endl << string(80, '=') << endl << endl; // Separator
    }

    print_header_box("Program Finished", 80);
    print_line_in_box(GREEN + " Execution completed. Thank you for using the program! " + RESET, 80, false, Alignment::Center);
    print_footer_box(80); cout << endl;

    return 0;
}