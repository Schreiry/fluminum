#pragma once
#include "Common.h"

// Helper to format coordinates for CSV axes
std::string format_coord(int n);

class Matrix {
public:
    // --- Constructors ---
    Matrix();
    Matrix(int rows, int cols);
    Matrix(int rows, int cols, double initialValue);
    Matrix(const std::vector<std::vector<double>>& data_2d);

    // --- Accessors ---
    int rows() const;
    int cols() const;
    bool isEmpty() const;
    size_t elementCount() const;

    // --- Operators ---
    double& operator()(int r, int c);
    double operator()(int r, int c) const;
    Matrix operator+(const Matrix& other) const;
    Matrix operator-(const Matrix& other) const;

    // --- Core Algorithms ---
    Matrix multiply_naive(const Matrix& other) const;
    long long compare_naive(const Matrix& other, double epsilon = 0.0) const;

    // --- Static Factory & Utility Methods ---
    static Matrix generateRandom(int rows, int cols);
    static Matrix identity(int n);
    static Matrix pad(const Matrix& A, int targetSize);
    static Matrix unpad(const Matrix& A, int originalRows, int originalCols);

    // --- Splitting and Combining for Strassen ---
    void split(Matrix& A11, Matrix& A12, Matrix& A21, Matrix& A22) const;
    static void split(const Matrix& A, const Matrix& B,
        Matrix& A11, Matrix& A12, Matrix& A21, Matrix& A22,
        Matrix& B11, Matrix& B12, Matrix& B21, Matrix& B22);
    static Matrix combine(const Matrix& C11, const Matrix& C12, const Matrix& C21, const Matrix& C22);

    // --- Public Member for Direct Data Access (if needed) ---
    const std::vector<double>& getRawData() const;

private:
    int rows_;
    int cols_;
    std::vector<double> data_;
};

// --- Helper Functions related to Matrix dimensions ---
int nextPowerOf2(int n);