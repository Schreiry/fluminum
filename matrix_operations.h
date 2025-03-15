// matrix_operations.h
#ifndef MATRIX_OPERATIONS_H
#define MATRIX_OPERATIONS_H

#include <vector>

const int STRASSEN_THRESHOLD = 32; // Threshold below which to use naive multiplication

std::vector<std::vector<int>> addMatrices(const std::vector<std::vector<int>>& A, const std::vector<std::vector<int>>& B);
std::vector<std::vector<int>> subtractMatrices(const std::vector<std::vector<int>>& A, const std::vector<std::vector<int>>& B);
void multiplyMatricesNaive(const std::vector<std::vector<int>>& A, const std::vector<std::vector<int>>& B, std::vector<std::vector<int>>& C);
void multiplyMatricesStrassen(const std::vector<std::vector<int>>& A, const std::vector<std::vector<int>>& B, std::vector<std::vector<int>>& C);
std::vector<std::vector<int>> getSubMatrix(const std::vector<std::vector<int>>& matrix, int rowStart, int colStart, int size);
void combineSubMatrices(const std::vector<std::vector<int>>& C11, const std::vector<std::vector<int>>& C12, const std::vector<std::vector<int>>& C21, const std::vector<std::vector<int>>& C22, std::vector<std::vector<int>>& C);
double calculateMatrixComplexity(const std::vector<std::vector<int>>& matrix);

#endif // MATRIX_OPERATIONS_H