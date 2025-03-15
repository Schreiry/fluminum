// matrix_operations.cpp
#include "matrix_operations.h"
#include <cmath>
#include <numeric> // For std::accumulate

double calculateMatrixComplexity(const std::vector<std::vector<int>>& matrix) {
    if (matrix.empty() || matrix[0].empty()) {
        return 0.0;
    }

    int rows = matrix.size();
    int cols = matrix[0].size();
    long long totalElements = static_cast<long long>(rows) * cols;

    // Factor 1: Average magnitude of numbers in the matrix
    double sumAbs = 0;
    for (const auto& row : matrix) {
        for (int val : row) {
            sumAbs += std::abs(val);
        }
    }
    double averageMagnitude = totalElements > 0 ? sumAbs / totalElements : 0;

    // Factor 2: Quantity of numbers (size) - can be directly used
    double sizeFactor = totalElements;

    // Factor 3: Probable number of numbers in the result (also size for square matrices)
    double resultSizeFactor = sizeFactor;

    // Combine these factors into a complexity coefficient (this is just an example)
    double complexity = std::log2(1 + averageMagnitude) * std::log2(1 + sizeFactor) * std::log2(1 + resultSizeFactor);

    return complexity;
}

std::vector<std::vector<int>> addMatrices(const std::vector<std::vector<int>>& A, const std::vector<std::vector<int>>& B) {
    int n = A.size();
    std::vector<std::vector<int>> result(n, std::vector<int>(n));
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            result[i][j] = A[i][j] + B[i][j];
        }
    }
    return result;
}

std::vector<std::vector<int>> subtractMatrices(const std::vector<std::vector<int>>& A, const std::vector<std::vector<int>>& B) {
    int n = A.size();
    std::vector<std::vector<int>> result(n, std::vector<int>(n));
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            result[i][j] = A[i][j] - B[i][j];
        }
    }
    return result;
}

void multiplyMatricesNaive(const std::vector<std::vector<int>>& A, const std::vector<std::vector<int>>& B, std::vector<std::vector<int>>& C) {
    int n = A.size();
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            int sum = 0;
            for (int k = 0; k < n; ++k) {
                sum += A[i][k] * B[k][j];
            }
            C[i][j] = sum;
        }
    }
}

std::vector<std::vector<int>> getSubMatrix(const std::vector<std::vector<int>>& matrix, int rowStart, int colStart, int size) {
    std::vector<std::vector<int>> sub(size, std::vector<int>(size));
    for (int i = 0; i < size; ++i) {
        for (int j = 0; j < size; ++j) {
            sub[i][j] = matrix[rowStart + i][colStart + j];
        }
    }
    return sub;
}

void combineSubMatrices(const std::vector<std::vector<int>>& C11, const std::vector<std::vector<int>>& C12, const std::vector<std::vector<int>>& C21, const std::vector<std::vector<int>>& C22, std::vector<std::vector<int>>& C) {
    int n2 = C11.size();
    int n = 2 * n2;
    for (int i = 0; i < n2; ++i) {
        for (int j = 0; j < n2; ++j) {
            C[i][j] = C11[i][j];
            C[i][j + n2] = C12[i][j];
            C[i + n2][j] = C21[i][j];
            C[i + n2][j + n2] = C22[i][j];
        }
    }
}

void multiplyMatricesStrassen(const std::vector<std::vector<int>>& A, const std::vector<std::vector<int>>& B, std::vector<std::vector<int>>& C) {
    int n = A.size();

    if (n <= STRASSEN_THRESHOLD) {
        multiplyMatricesNaive(A, B, C);
        return;
    }

    int n2 = n / 2;
    std::vector<std::vector<int>> A11 = getSubMatrix(A, 0, 0, n2);
    std::vector<std::vector<int>> A12 = getSubMatrix(A, 0, n2, n2);
    std::vector<std::vector<int>> A21 = getSubMatrix(A, n2, 0, n2);
    std::vector<std::vector<int>> A22 = getSubMatrix(A, n2, n2, n2);
    std::vector<std::vector<int>> B11 = getSubMatrix(B, 0, 0, n2);
    std::vector<std::vector<int>> B12 = getSubMatrix(B, 0, n2, n2);
    std::vector<std::vector<int>> B21 = getSubMatrix(B, n2, 0, n2);
    std::vector<std::vector<int>> B22 = getSubMatrix(B, n2, n2, n2);

    std::vector<std::vector<int>> P1(n2, std::vector<int>(n2));
    std::vector<std::vector<int>> P2(n2, std::vector<int>(n2));
    std::vector<std::vector<int>> P3(n2, std::vector<int>(n2));
    std::vector<std::vector<int>> P4(n2, std::vector<int>(n2));
    std::vector<std::vector<int>> P5(n2, std::vector<int>(n2));
    std::vector<std::vector<int>> P6(n2, std::vector<int>(n2));
    std::vector<std::vector<int>> P7(n2, std::vector<int>(n2));

    std::vector<std::vector<int>> tempA(n2, std::vector<int>(n2));
    std::vector<std::vector<int>> tempB(n2, std::vector<int>(n2));

    // P1 = A11 * (B12 - B22)
    tempB = subtractMatrices(B12, B22);
    multiplyMatricesStrassen(A11, tempB, P1);

    // P2 = (A11 + A12) * B22
    tempA = addMatrices(A11, A12);
    multiplyMatricesStrassen(tempA, B22, P2);

    // P3 = (A21 + A22) * B11
    tempA = addMatrices(A21, A22);
    multiplyMatricesStrassen(tempA, B11, P3);

    // P4 = A22 * (B21 - B11)
    tempB = subtractMatrices(B21, B11);
    multiplyMatricesStrassen(A22, tempB, P4);

    // P5 = (A11 + A22) * (B11 + B22)
    tempA = addMatrices(A11, A22);
    tempB = addMatrices(B11, B22);
    multiplyMatricesStrassen(tempA, tempB, P5);

    // P6 = (A12 - A22) * (B21 + B22)
    tempA = subtractMatrices(A12, A22);
    tempB = addMatrices(B21, B22);
    multiplyMatricesStrassen(tempA, tempB, P6);

    // P7 = (A11 - A21) * (B11 + B12)
    tempA = subtractMatrices(A11, A21);
    tempB = addMatrices(B11, B12);
    multiplyMatricesStrassen(tempA, tempB, P7);

    std::vector<std::vector<int>> C11(n2, std::vector<int>(n2));
    std::vector<std::vector<int>> C12(n2, std::vector<int>(n2));
    std::vector<std::vector<int>> C21(n2, std::vector<int>(n2));
    std::vector<std::vector<int>> C22(n2, std::vector<int>(n2));

    // C11 = P5 + P4 - P2 + P6
    tempA = addMatrices(P5, P4);
    tempB = subtractMatrices(tempA, P2);
    C11 = addMatrices(tempB, P6);

    // C12 = P1 + P2
    C12 = addMatrices(P1, P2);

    // C21 = P3 + P4
    C21 = addMatrices(P3, P4);

    // C22 = P5 + P1 - P3 - P7
    tempA = addMatrices(P5, P1);
    tempB = subtractMatrices(tempA, P3);
    C22 = subtractMatrices(tempB, P7);

    combineSubMatrices(C11, C12, C21, C22, C);
}