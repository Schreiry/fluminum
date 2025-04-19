// matrix_cache_optimizer.h
#ifndef MATRIX_CACHE_OPTIMIZER_H
#define MATRIX_CACHE_OPTIMIZER_H

#include <vector>
#include <cstdint>
#include <functional>

// Compression modes for matrix data
enum class CompressionMode {
    NONE,           // No compression
    RLE,            // Run-length encoding for repeated values
    BITPACK,        // Bit-packing for small integer values
    SPARSE,         // Sparse representation for matrices with many zeros
    ADAPTIVE        // Automatically select the best method
};

// Structure to hold compressed matrix data
struct CompressedMatrix {
    size_t originalRows;
    size_t originalCols;
    CompressionMode mode;
    std::vector<uint8_t> compressedData;
    std::vector<size_t> metadata; // Additional data needed for decompression
};

// Cache-optimized matrix that manages its own memory layout
class CacheOptimizedMatrix {
private:
    std::vector<int> data;
    size_t rows;
    size_t cols;
    size_t blockSize; // For cache-efficient blocking
    bool isCompressed;
    CompressedMatrix compressed;

    // Internal helpers
    void optimizeDataLayout();
    void blockForCache();

public:
    CacheOptimizedMatrix(size_t r, size_t c, int defaultValue = 0);
    CacheOptimizedMatrix(const std::vector<std::vector<int>>& source);
    
    // Convert to/from standard vector format
    std::vector<std::vector<int>> toStandardMatrix() const;
    void fromStandardMatrix(const std::vector<std::vector<int>>& source);
    
    // Cache-efficient element access
    int get(size_t i, size_t j) const;
    void set(size_t i, size_t j, int value);
    
    // Compression methods
    void compress(CompressionMode mode = CompressionMode::ADAPTIVE);
    void decompress();
    
    // Properties
    size_t getRows() const { return rows; }
    size_t getCols() const { return cols; }
    bool isInCompressedState() const { return isCompressed; }
    size_t memoryUsage() const;
    
    // Optimized matrix operations specially designed for cache efficiency
    void addTo(const CacheOptimizedMatrix& other, CacheOptimizedMatrix& result) const;
    void subtractFrom(const CacheOptimizedMatrix& other, CacheOptimizedMatrix& result) const;
    void multiplyBy(const CacheOptimizedMatrix& other, CacheOptimizedMatrix& result) const;
    
    // Block operations for parallel processing
    CacheOptimizedMatrix getBlock(size_t startRow, size_t startCol, size_t blockRows, size_t blockCols) const;
    void setBlock(size_t startRow, size_t startCol, const CacheOptimizedMatrix& block);
};

// Helper functions
CompressedMatrix compressMatrixRLE(const std::vector<int>& data, size_t rows, size_t cols);
CompressedMatrix compressMatrixBitpack(const std::vector<int>& data, size_t rows, size_t cols);
CompressedMatrix compressMatrixSparse(const std::vector<int>& data, size_t rows, size_t cols);
CompressedMatrix selectBestCompression(const std::vector<int>& data, size_t rows, size_t cols);
std::vector<int> decompressMatrix(const CompressedMatrix& compressed);

#endif // MATRIX_CACHE_OPTIMIZER_H
