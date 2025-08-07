
#define NOMINMAX
#include "Matrix.h"
#include "System.h" // For getSystemMemoryInfo in nextPowerOf2

// Helper to format coordinates for CSV axes
std::string format_coord(int n) {
    std::stringstream ss;
    ss << std::setw(4) << std::setfill('0') << n;
    return ss.str();
}

// --- Constructors ---
Matrix::Matrix() : rows_(0), cols_(0) {}

Matrix::Matrix(int rows, int cols) : rows_(rows), cols_(cols) {
    if (rows < 0 || cols < 0) throw std::invalid_argument("Matrix dimensions cannot be negative.");
    unsigned long long numElements_ull = static_cast<unsigned long long>(rows) * static_cast<unsigned long long>(cols);
    if (rows > 0 && cols > 0 && numElements_ull / static_cast<unsigned long long>(rows) != static_cast<unsigned long long>(cols)) {
        throw std::bad_alloc(); // Overflow check
    }
    if (numElements_ull > std::vector<double>().max_size()) {
        throw std::bad_alloc();
    }
    data_.resize(static_cast<size_t>(numElements_ull), 0.0);
}

Matrix::Matrix(int rows, int cols, double initialValue) : rows_(rows), cols_(cols) {
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

Matrix::Matrix(const std::vector<std::vector<double>>& data_2d) {
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

// --- Accessors ---
int Matrix::rows() const { return rows_; }
int Matrix::cols() const { return cols_; }
bool Matrix::isEmpty() const { return rows_ == 0 || cols_ == 0; }
size_t Matrix::elementCount() const { return static_cast<size_t>(rows_) * static_cast<size_t>(cols_); }

// --- Operators ---
double& Matrix::operator()(int r, int c) {
    if (r < 0 || r >= rows_ || c < 0 || c >= cols_) throw std::out_of_range("Matrix index out of range.");
    return data_[static_cast<size_t>(r) * cols_ + c];
}

double Matrix::operator()(int r, int c) const {
    if (r < 0 || r >= rows_ || c < 0 || c >= cols_) throw std::out_of_range("Matrix index out of range.");
    return data_[static_cast<size_t>(r) * cols_ + c];
}

Matrix Matrix::operator+(const Matrix& other) const {
    if (rows_ != other.rows_ || cols_ != other.cols_) throw std::invalid_argument("Matrix dimensions must match for addition.");
    Matrix result(rows_, cols_);
    for (size_t i = 0; i < data_.size(); ++i) result.data_[i] = data_[i] + other.data_[i];
    return result;
}

Matrix Matrix::operator-(const Matrix& other) const {
    if (rows_ != other.rows_ || cols_ != other.cols_) throw std::invalid_argument("Matrix dimensions must match for subtraction.");
    Matrix result(rows_, cols_);
    for (size_t i = 0; i < data_.size(); ++i) result.data_[i] = data_[i] - other.data_[i];
    return result;
}

// --- Core Algorithms ---
Matrix Matrix::multiply_naive(const Matrix& other) const {
    if (cols_ != other.rows_) throw std::invalid_argument("Matrix dimensions incompatible for multiplication (A.cols != B.rows).");
    if (rows_ == 0 || cols_ == 0 || other.cols_ == 0) return Matrix(rows_, other.cols_);

    Matrix result(rows_, other.cols_);
    int M = rows_; int N = cols_; int P = other.cols_;
    const double* a_ptr = data_.data();
    const double* b_ptr = other.data_.data();
    double* c_ptr = result.data_.data();

#ifdef HAS_AVX
    if (has_avx_global && N > 0 && P >= SIMD_VECTOR_SIZE_DOUBLE) {
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
    if (has_sse2_global && N > 0 && P >= SIMD_VECTOR_SIZE_DOUBLE && !has_avx_global) {
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
    // Fallback scalar implementation
    for (int i = 0; i < M; ++i) {
        for (int j = 0; j < P; ++j) {
            double sum = 0.0;
            for (int k = 0; k < N; ++k) sum += (*this)(i, k) * other(k, j);
            result(i, j) = sum;
        }
    }
    return result;
}

long long Matrix::compare_naive(const Matrix& other, double epsilon) const {
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

// --- Static Factory & Utility Methods ---
Matrix Matrix::generateRandom(int rows, int cols) {
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

Matrix Matrix::identity(int n) {
    if (n <= 0) throw std::invalid_argument("Identity matrix dimension must be positive.");
    Matrix result(n, n);
    for (int i = 0; i < n; ++i) result(i, i) = 1.0;
    return result;
}

Matrix Matrix::pad(const Matrix& A, int targetSize) {
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

Matrix Matrix::unpad(const Matrix& A, int originalRows, int originalCols) {
    if (originalRows == A.rows() && originalCols == A.cols()) return A;
    if (originalRows == 0 || originalCols == 0) {
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

// --- Splitting and Combining for Strassen ---
void Matrix::split(Matrix& A11, Matrix& A12, Matrix& A21, Matrix& A22) const {
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

void Matrix::split(const Matrix& A, const Matrix& B,
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
            A21(i, j) = A(i + n2, j);   A22(i, j) = A(i + n2, j + n2);
            B11(i, j) = B(i, j);       B12(i, j) = B(i, j + n2);
            B21(i, j) = B(i + n2, j);   B22(i, j) = B(i + n2, j + n2);
        }
    }
}

Matrix Matrix::combine(const Matrix& C11, const Matrix& C12, const Matrix& C21, const Matrix& C22) {
    int n2 = C11.rows();
    if (C11.cols() != n2 || C12.rows() != n2 || C12.cols() != n2 ||
        C21.rows() != n2 || C21.cols() != n2 || C22.rows() != n2 || C22.cols() != n2 || n2 == 0)
        throw std::invalid_argument("Quadrants for combining must be non-empty, square, and same dimensions.");
    int n = n2 * 2;
    Matrix C(n, n);
    for (int i = 0; i < n2; ++i) {
        for (int j = 0; j < n2; ++j) {
            C(i, j) = C11(i, j);        C(i, j + n2) = C12(i, j);
            C(i + n2, j) = C21(i, j);   C(i + n2, j + n2) = C22(i, j);
        }
    }
    return C;
}


const std::vector<double>& Matrix::getRawData() const {
    return data_;
}


// --- Helper Functions related to Matrix dimensions ---
int nextPowerOf2(int n) {
    if (n <= 0) return 1;
    if (n == 1) return 1;

    // Simplified check, assuming typical system limits.
    // The original checks were very thorough but complex.
    if (n > 65536) { // A very large dimension that would require > 100GB RAM
        SystemMemoryInfo memInfo = getSystemMemoryInfo();
        // Estimate memory for just three padded matrices
        unsigned long long required_mem_mb = 3ULL * n * n * sizeof(double) / (1024 * 1024);
        if (memInfo.totalPhysicalMB > 0 && required_mem_mb > memInfo.totalPhysicalMB) {
            throw std::overflow_error("Input dimension " + std::to_string(n) + " is impractically large for system RAM.");
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