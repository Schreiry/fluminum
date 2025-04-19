// matrix_cache_optimizer.cpp
#include "matrix_cache_optimizer.h"
#include <algorithm>
#include <numeric>
#include <cmath>
#include <limits>
#include <cassert>
#include <iostream>

// Optimal cache block size - adjust based on L1 cache size (typically 32KB or 64KB)
// A typical cache line is 64 bytes, so 16 integers (4 bytes each) fit in one line
const size_t DEFAULT_BLOCK_SIZE = 64;

CacheOptimizedMatrix::CacheOptimizedMatrix(size_t r, size_t c, int defaultValue) 
    : rows(r), cols(c), isCompressed(false) {
    // Round up block size to nearest power of 2 that divides matrix dimensions well
    blockSize = DEFAULT_BLOCK_SIZE;
    while (blockSize > 8 && (r % blockSize != 0 || c % blockSize != 0)) {
        blockSize /= 2;
    }
    
    // Allocate memory with cache-friendly layout
    data.resize(r * c, defaultValue);
    optimizeDataLayout();
}

CacheOptimizedMatrix::CacheOptimizedMatrix(const std::vector<std::vector<int>>& source)
    : isCompressed(false) {
    if (source.empty()) {
        rows = 0;
        cols = 0;
        return;
    }
    
    rows = source.size();
    cols = source[0].size();
    
    // Determine optimal block size
    blockSize = DEFAULT_BLOCK_SIZE;
    while (blockSize > 8 && (rows % blockSize != 0 || cols % blockSize != 0)) {
        blockSize /= 2;
    }
    
    // Copy data with cache-friendly layout
    data.resize(rows * cols);
    fromStandardMatrix(source);
}

void CacheOptimizedMatrix::optimizeDataLayout() {
    // Z-order curve or Morton order for improved cache locality
    // This is a simplified version - a full implementation would rearrange data
    // For now, we'll use block-based layout which is more practical
    blockForCache();
}

void CacheOptimizedMatrix::blockForCache() {
    // Implemented through access patterns in get() and set()
    // The actual data stays in row-major order, but access is optimized
}

int CacheOptimizedMatrix::get(size_t i, size_t j) const {
    if (isCompressed) {
        // We need to decompress for random access
        // In a full implementation, we might decompress just the needed block
        const_cast<CacheOptimizedMatrix*>(this)->decompress();
    }
    
    // Cache-optimized access
    size_t blockRow = i / blockSize;
    size_t blockCol = j / blockSize;
    size_t localRow = i % blockSize;
    size_t localCol = j % blockSize;
    
    // Compute index using block-based layout
    size_t blockIndex = (blockRow * (cols / blockSize) + blockCol) * (blockSize * blockSize);
    size_t localIndex = localRow * blockSize + localCol;
    
    return data[blockIndex + localIndex];
}

void CacheOptimizedMatrix::set(size_t i, size_t j, int value) {
    if (isCompressed) {
        decompress();
    }
    
    // Cache-optimized access using the same block-based indexing
    size_t blockRow = i / blockSize;
    size_t blockCol = j / blockSize;
    size_t localRow = i % blockSize;
    size_t localCol = j % blockSize;
    
    // Compute index using block-based layout
    size_t blockIndex = (blockRow * (cols / blockSize) + blockCol) * (blockSize * blockSize);
    size_t localIndex = localRow * blockSize + localCol;
    
    data[blockIndex + localIndex] = value;
}

std::vector<std::vector<int>> CacheOptimizedMatrix::toStandardMatrix() const {
    // Create a standard 2D vector representation
    std::vector<std::vector<int>> result(rows, std::vector<int>(cols));
    
    // If compressed, decompress first
    if (isCompressed) {
        const_cast<CacheOptimizedMatrix*>(this)->decompress();
    }
    
    // Copy data while respecting the layout
    for (size_t i = 0; i < rows; ++i) {
        for (size_t j = 0; j < cols; ++j) {
            result[i][j] = get(i, j);
        }
    }
    
    return result;
}

void CacheOptimizedMatrix::fromStandardMatrix(const std::vector<std::vector<int>>& source) {
    rows = source.size();
    cols = source.empty() ? 0 : source[0].size();
    
    // Resize and copy data
    data.resize(rows * cols);
    isCompressed = false;
    
    // Determine optimal block size
    blockSize = DEFAULT_BLOCK_SIZE;
    while (blockSize > 8 && (rows % blockSize != 0 || cols % blockSize != 0)) {
        blockSize /= 2;
    }
    
    // Copy data using the set method to handle the layout transformation
    for (size_t i = 0; i < rows; ++i) {
        for (size_t j = 0; j < cols; ++j) {
            set(i, j, source[i][j]);
        }
    }
}

void CacheOptimizedMatrix::compress(CompressionMode mode) {
    if (isCompressed) return;
    
    // Select compression method
    if (mode == CompressionMode::ADAPTIVE) {
        compressed = selectBestCompression(data, rows, cols);
    } else if (mode == CompressionMode::RLE) {
        compressed = compressMatrixRLE(data, rows, cols);
    } else if (mode == CompressionMode::BITPACK) {
        compressed = compressMatrixBitpack(data, rows, cols);
    } else if (mode == CompressionMode::SPARSE) {
        compressed = compressMatrixSparse(data, rows, cols);
    } else {
        // No compression
        return;
    }
    
    compressed.originalRows = rows;
    compressed.originalCols = cols;
    compressed.mode = mode;
    
    // If compression was successful, mark as compressed and clear original data
    if (!compressed.compressedData.empty()) {
        isCompressed = true;
        std::vector<int>().swap(data); // Free memory used by uncompressed data
    }
}

void CacheOptimizedMatrix::decompress() {
    if (!isCompressed) return;
    
    // Restore data from compressed form
    data = decompressMatrix(compressed);
    rows = compressed.originalRows;
    cols = compressed.originalCols;
    
    // Clear compressed data and mark as uncompressed
    CompressedMatrix().swap(compressed);
    isCompressed = false;
    
    // Reoptimize the data layout
    optimizeDataLayout();
}

size_t CacheOptimizedMatrix::memoryUsage() const {
    if (isCompressed) {
        return compressed.compressedData.size() * sizeof(uint8_t) +
               compressed.metadata.size() * sizeof(size_t) +
               sizeof(CompressedMatrix);
    } else {
        return data.size() * sizeof(int) + sizeof(CacheOptimizedMatrix);
    }
}

void CacheOptimizedMatrix::addTo(const CacheOptimizedMatrix& other, CacheOptimizedMatrix& result) const {
    // Ensure dimensions match
    assert(rows == other.rows && cols == other.cols);
    
    // Decompress if needed
    if (isCompressed) {
        const_cast<CacheOptimizedMatrix*>(this)->decompress();
    }
    if (other.isCompressed) {
        const_cast<CacheOptimizedMatrix&>(other).decompress();
    }
    
    // Resize result
    if (result.rows != rows || result.cols != cols) {
        result = CacheOptimizedMatrix(rows, cols);
    } else if (result.isCompressed) {
        result.decompress();
    }
    
    // Add matrices block by block for cache efficiency
    for (size_t i = 0; i < rows; i += blockSize) {
        for (size_t j = 0; j < cols; j += blockSize) {
            // Process each block
            for (size_t bi = 0; bi < blockSize && i + bi < rows; ++bi) {
                for (size_t bj = 0; bj < blockSize && j + bj < cols; ++bj) {
                    int sum = get(i + bi, j + bj) + other.get(i + bi, j + bj);
                    result.set(i + bi, j + bj, sum);
                }
            }
        }
    }
}

void CacheOptimizedMatrix::subtractFrom(const CacheOptimizedMatrix& other, CacheOptimizedMatrix& result) const {
    // Ensure dimensions match
    assert(rows == other.rows && cols == other.cols);
    
    // Decompress if needed
    if (isCompressed) {
        const_cast<CacheOptimizedMatrix*>(this)->decompress();
    }
    if (other.isCompressed) {
        const_cast<CacheOptimizedMatrix&>(other).decompress();
    }
    
    // Resize result
    if (result.rows != rows || result.cols != cols) {
        result = CacheOptimizedMatrix(rows, cols);
    } else if (result.isCompressed) {
        result.decompress();
    }
    
    // Subtract matrices block by block for cache efficiency
    for (size_t i = 0; i < rows; i += blockSize) {
        for (size_t j = 0; j < cols; j += blockSize) {
            // Process each block
            for (size_t bi = 0; bi < blockSize && i + bi < rows; ++bi) {
                for (size_t bj = 0; bj < blockSize && j + bj < cols; ++bj) {
                    int diff = get(i + bi, j + bj) - other.get(i + bi, j + bj);
                    result.set(i + bi, j + bj, diff);
                }
            }
        }
    }
}

void CacheOptimizedMatrix::multiplyBy(const CacheOptimizedMatrix& other, CacheOptimizedMatrix& result) const {
    // Ensure dimensions are compatible for multiplication
    assert(cols == other.rows);
    
    // Decompress if needed
    if (isCompressed) {
        const_cast<CacheOptimizedMatrix*>(this)->decompress();
    }
    if (other.isCompressed) {
        const_cast<CacheOptimizedMatrix&>(other).decompress();
    }
    
    // Resize result
    if (result.rows != rows || result.cols != other.cols) {
        result = CacheOptimizedMatrix(rows, other.cols);
    } else if (result.isCompressed) {
        result.decompress();
    }
    
    // Cache-optimized block matrix multiplication
    for (size_t i = 0; i < rows; i += blockSize) {
        for (size_t j = 0; j < other.cols; j += blockSize) {
            // Initialize result block to zeros
            for (size_t bi = 0; bi < blockSize && i + bi < rows; ++bi) {
                for (size_t bj = 0; bj < blockSize && j + bj < other.cols; ++bj) {
                    result.set(i + bi, j + bj, 0);
                }
            }
            
            // Multiply blocks
            for (size_t k = 0; k < cols; k += blockSize) {
                for (size_t bi = 0; bi < blockSize && i + bi < rows; ++bi) {
                    for (size_t bj = 0; bj < blockSize && j + bj < other.cols; ++bj) {
                        int sum = result.get(i + bi, j + bj);
                        for (size_t bk = 0; bk < blockSize && k + bk < cols; ++bk) {
                            sum += get(i + bi, k + bk) * other.get(k + bk, j + bj);
                        }
                        result.set(i + bi, j + bj, sum);
                    }
                }
            }
        }
    }
}

CacheOptimizedMatrix CacheOptimizedMatrix::getBlock(size_t startRow, size_t startCol, size_t blockRows, size_t blockCols) const {
    // Validate boundaries
    assert(startRow + blockRows <= rows && startCol + blockCols <= cols);
    
    // Create result matrix
    CacheOptimizedMatrix result(blockRows, blockCols);
    
    // Decompress if needed for access
    if (isCompressed) {
        const_cast<CacheOptimizedMatrix*>(this)->decompress();
    }
    
    // Copy block data
    for (size_t i = 0; i < blockRows; ++i) {
        for (size_t j = 0; j < blockCols; ++j) {
            result.set(i, j, get(startRow + i, startCol + j));
        }
    }
    
    return result;
}

void CacheOptimizedMatrix::setBlock(size_t startRow, size_t startCol, const CacheOptimizedMatrix& block) {
    // Validate boundaries
    size_t blockRows = block.getRows();
    size_t blockCols = block.getCols();
    assert(startRow + blockRows <= rows && startCol + blockCols <= cols);
    
    // Decompress if needed for modification
    if (isCompressed) {
        decompress();
    }
    if (block.isCompressed) {
        const_cast<CacheOptimizedMatrix&>(block).decompress();
    }
    
    // Copy block data
    for (size_t i = 0; i < blockRows; ++i) {
        for (size_t j = 0; j < blockCols; ++j) {
            set(startRow + i, startCol + j, block.get(i, j));
        }
    }
}

// Compression implementations

CompressedMatrix compressMatrixRLE(const std::vector<int>& data, size_t rows, size_t cols) {
    CompressedMatrix result;
    result.originalRows = rows;
    result.originalCols = cols;
    result.mode = CompressionMode::RLE;
    
    if (data.empty()) return result;
    
    // Run-length encoding
    int currentValue = data[0];
    int count = 1;
    
    // Reserve space for the worst case (no compression)
    result.compressedData.reserve(data.size() * sizeof(int) + data.size() / 2);
    
    auto appendInt = [&result](int value) {
        uint8_t* bytes = reinterpret_cast<uint8_t*>(&value);
        for (size_t i = 0; i < sizeof(int); ++i) {
            result.compressedData.push_back(bytes[i]);
        }
    };
    
    // Process all elements
    for (size_t i = 1; i < data.size(); ++i) {
        if (data[i] == currentValue && count < 255) {
            // Continue current run
            count++;
        } else {
            // End current run and start a new one
            result.compressedData.push_back(static_cast<uint8_t>(count));
            appendInt(currentValue);
            
            currentValue = data[i];
            count = 1;
        }
    }
    
    // Add the last run
    result.compressedData.push_back(static_cast<uint8_t>(count));
    appendInt(currentValue);
    
    return result;
}

CompressedMatrix compressMatrixBitpack(const std::vector<int>& data, size_t rows, size_t cols) {
    CompressedMatrix result;
    result.originalRows = rows;
    result.originalCols = cols;
    result.mode = CompressionMode::BITPACK;
    
    if (data.empty()) return result;
    
    // Find the range of values to determine bits needed
    int minVal = *std::min_element(data.begin(), data.end());
    int maxVal = *std::max_element(data.begin(), data.end());
    
    // Store min and max in metadata
    result.metadata.push_back(minVal);
    result.metadata.push_back(maxVal);
    
    // Calculate bits needed per value
    uint32_t range = maxVal - minVal;
    uint8_t bitsPerValue = range <= 0 ? 1 : std::ceil(std::log2(range + 1));
    
    // Store bits per value in metadata
    result.metadata.push_back(bitsPerValue);
    
    // No compression benefit if we need more than 16 bits
    if (bitsPerValue > 16) {
        // Just copy the data as bytes
        result.compressedData.resize(data.size() * sizeof(int));
        std::memcpy(result.compressedData.data(), data.data(), data.size() * sizeof(int));
        return result;
    }
    
    // Pack values into bits
    result.compressedData.reserve((data.size() * bitsPerValue + 7) / 8 + sizeof(int));
    
    uint32_t buffer = 0;
    uint8_t bitsInBuffer = 0;
    
    for (int val : data) {
        // Normalize value to the range [0, range]
        uint32_t normalizedVal = val - minVal;
        
        // Add bits to buffer
        buffer |= (normalizedVal << bitsInBuffer);
        bitsInBuffer += bitsPerValue;
        
        // Extract bytes when we have enough bits
        while (bitsInBuffer >= 8) {
            result.compressedData.push_back(buffer & 0xFF);
            buffer >>= 8;
            bitsInBuffer -= 8;
        }
    }
    
    // Add any remaining bits in the buffer
    if (bitsInBuffer > 0) {
        result.compressedData.push_back(buffer & 0xFF);
    }
    
    return result;
}

CompressedMatrix compressMatrixSparse(const std::vector<int>& data, size_t rows, size_t cols) {
    CompressedMatrix result;
    result.originalRows = rows;
    result.originalCols = cols;
    result.mode = CompressionMode::SPARSE;
    
    if (data.empty()) return result;
    
    // Count non-zero elements
    int nonZeroCount = std::count_if(data.begin(), data.end(), [](int val) { return val != 0; });
    
    // If less than 25% elements are non-zero, sparse format is efficient
    if (nonZeroCount > data.size() / 4) {
        // Not sparse enough, return empty result to indicate no compression
        return result;
    }
    
    // Allocate space for indices and values
    result.metadata.push_back(nonZeroCount);
    result.compressedData.reserve(nonZeroCount * (sizeof(int) + sizeof(size_t)));
    
    // Store non-zero elements with their indices
    for (size_t i = 0; i < data.size(); ++i) {
        if (data[i] != 0) {
            // Store index
            size_t index = i;
            uint8_t* indexBytes = reinterpret_cast<uint8_t*>(&index);
            for (size_t b = 0; b < sizeof(size_t); ++b) {
                result.compressedData.push_back(indexBytes[b]);
            }
            
            // Store value
            int value = data[i];
            uint8_t* valueBytes = reinterpret_cast<uint8_t*>(&value);
            for (size_t b = 0; b < sizeof(int); ++b) {
                result.compressedData.push_back(valueBytes[b]);
            }
        }
    }
    
    return result;
}

CompressedMatrix selectBestCompression(const std::vector<int>& data, size_t rows, size_t cols) {
    if (data.empty()) {
        CompressedMatrix empty;
        empty.originalRows = rows;
        empty.originalCols = cols;
        empty.mode = CompressionMode::NONE;
        return empty;
    }
    
    // Try different compression methods and select the best one
    CompressedMatrix rleCompressed = compressMatrixRLE(data, rows, cols);
    CompressedMatrix bitpackCompressed = compressMatrixBitpack(data, rows, cols);
    CompressedMatrix sparseCompressed = compressMatrixSparse(data, rows, cols);
    
    // Calculate original size
    size_t originalSize = data.size() * sizeof(int);
    
    // Find the smallest compression
    size_t rleSize = rleCompressed.compressedData.size();
    size_t bitpackSize = bitpackCompressed.compressedData.size() + bitpackCompressed.metadata.size() * sizeof(size_t);
    size_t sparseSize = sparseCompressed.compressedData.size() + sizeof(size_t);
    
    // Choose the best compression or none if compression doesn't save space
    if (rleSize <= bitpackSize && rleSize <= sparseSize && rleSize < originalSize * 0.9) {
        return rleCompressed;
    } else if (bitpackSize <= rleSize && bitpackSize <= sparseSize && bitpackSize < originalSize * 0.9) {
        return bitpackCompressed;
    } else if (sparseSize <= rleSize && sparseSize <= bitpackSize && sparseSize < originalSize * 0.9) {
        return sparseCompressed;
    } else {
        // No compression is better
        CompressedMatrix noCompression;
        noCompression.originalRows = rows;
        noCompression.originalCols = cols;
        noCompression.mode = CompressionMode::NONE;
        return noCompression;
    }
}

std::vector<int> decompressMatrix(const CompressedMatrix& compressed) {
    std::vector<int> result;
    size_t rows = compressed.originalRows;
    size_t cols = compressed.originalCols;
    size_t totalSize = rows * cols;
    
    if (compressed.mode == CompressionMode::NONE || compressed.compressedData.empty()) {
        // No compression was applied
        if (compressed.compressedData.size() == totalSize * sizeof(int)) {
            // Data was stored directly as int bytes
            result.resize(totalSize);
            std::memcpy(result.data(), compressed.compressedData.data(), totalSize * sizeof(int));
        } else {
            // Initialize to zeros
            result.resize(totalSize, 0);
        }
        return result;
    }
    
    result.reserve(totalSize);
    
    if (compressed.mode == CompressionMode::RLE) {
        // Run-length decoding
        size_t i = 0;
        while (i < compressed.compressedData.size()) {
            uint8_t count = compressed.compressedData[i++];
            
            // Read the int value (4 bytes)
            int value = 0;
            if (i + sizeof(int) <= compressed.compressedData.size()) {
                std::memcpy(&value, &compressed.compressedData[i], sizeof(int));
                i += sizeof(int);
            }
            
            // Repeat the value
            for (uint8_t j = 0; j < count && result.size() < totalSize; ++j) {
                result.push_back(value);
            }
        }
    }
    else if (compressed.mode == CompressionMode::BITPACK) {
        // Check if we have the necessary metadata
        if (compressed.metadata.size() < 3) {
            result.resize(totalSize, 0);
            return result;
        }
        
        int minVal = compressed.metadata[0];
        int maxVal = compressed.metadata[1];
        uint8_t bitsPerValue = compressed.metadata[2];
        
        if (bitsPerValue > 16) {
            // Data was stored directly
            result.resize(totalSize);
            std::memcpy(result.data(), compressed.compressedData.data(), totalSize * sizeof(int));
            return result;
        }
        
        // Unpack bits
        uint64_t bitBuffer = 0;
        uint8_t bitsInBuffer = 0;
        size_t byteIndex = 0;
        
        for (size_t i = 0; i < totalSize; ++i) {
            // Ensure we have enough bits in the buffer
            while (bitsInBuffer < bitsPerValue && byteIndex < compressed.compressedData.size()) {
                bitBuffer |= (static_cast<uint64_t>(compressed.compressedData[byteIndex++]) << bitsInBuffer);
                bitsInBuffer += 8;
            }
            
            // Extract value using a mask
            uint32_t mask = (1ULL << bitsPerValue) - 1;
            uint32_t normalizedVal = bitBuffer & mask;
            bitBuffer >>= bitsPerValue;
            bitsInBuffer -= bitsPerValue;
            
            // Denormalize and add to result
            result.push_back(normalizedVal + minVal);
        }
    }
    else if (compressed.mode == CompressionMode::SPARSE) {
        // Initialize with zeros
        result.resize(totalSize, 0);
        
        if (compressed.metadata.empty()) {
            return result;
        }
        
        size_t nonZeroCount = compressed.metadata[0];
        size_t entrySize = sizeof(size_t) + sizeof(int);
        
        for (size_t i = 0; i < nonZeroCount && i * entrySize < compressed.compressedData.size(); ++i) {
            size_t index;
            int value;
            size_t offset = i * entrySize;
            
            // Read index
            if (offset + sizeof(size_t) <= compressed.compressedData.size()) {
                std::memcpy(&index, &compressed.compressedData[offset], sizeof(size_t));
            } else {
                break;
            }
            
            // Read value
            if (offset + sizeof(size_t) + sizeof(int) <= compressed.compressedData.size()) {
                std::memcpy(&value, &compressed.compressedData[offset + sizeof(size_t)], sizeof(int));
            } else {
                break;
            }
            
            // Set the value if index is valid
            if (index < totalSize) {
                result[index] = value;
            }
        }
    }
    
    // Ensure we have the correct number of elements
    if (result.size() < totalSize) {
        result.resize(totalSize, 0);
    } else if (result.size() > totalSize) {
        result.resize(totalSize);
    }
    
    return result;
}
