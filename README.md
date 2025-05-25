# Fluminum: High-Performance Matrix Multiplication with Strassen's Algorithm

## Project Overview

Fluminum represents a cutting-edge implementation of optimized matrix multiplication, leveraging Strassen's divide-and-conquer algorithm combined with advanced parallelization techniques. This project demonstrates a revolutionary approach to large-scale matrix operations, achieving performance improvements of up to **574x** over traditional naive multiplication methods.

The implementation transcends simple algorithmic optimization by incorporating sophisticated multi-threading, intelligent hardware adaptation, and cache-efficient memory management. Written in modern C++, Fluminum automatically adapts to system hardware capabilities, dynamically calculating optimal thread distribution and workload balancing.

## Key Features

### Core Algorithm Implementation
- **Strassen's Algorithm**: Reduces computational complexity from O(n³) to O(n^2.807)
- **Adaptive Threshold System**: Intelligently switches between Strassen and naive multiplication based on matrix size
- **Recursive Optimization**: Efficient handling of matrix subdivision and recombination

### Advanced Parallelization
- **Multi-Threading Framework**: Full utilization of modern multi-core processors
- **Dynamic Thread Management**: Automatic calculation of optimal thread count based on system capabilities
- **Load Balancing**: Intelligent distribution of computational workload across available cores
- **Synchronization Primitives**: Thread-safe operations using `std::mutex`, `std::condition_variable`, and `std::atomic`

### System Intelligence
- **Hardware Detection**: Comprehensive CPU information gathering (name, cores, threads)
- **Cross-Platform Compatibility**: Native support for Windows and Linux environments
- **Performance Monitoring**: Real-time computation tracking and performance analytics

## Installation and Setup

### Prerequisites
- **C++ Compiler**: C++11 or later (GCC, Clang, or MSVC)
- **Operating System**: Windows 10/11 or Linux (Ubuntu 18.04+, CentOS 7+)
- **Hardware**: Multi-core CPU recommended (tested extensively on Intel and AMD architectures)

### Installation Steps

```bash
# Clone the repository
git clone https://github.com/Schreiry/fluminum.git

# Navigate to project directory
cd fluminum

# Compile the project
g++ -std=c++11 -O3 -o matrix_multiply Flens.cpp system_info.cpp matrix_operations.cpp -lpthread

# For Windows with MinGW
g++ -std=c++11 -O3 -o matrix_multiply.exe Flens.cpp system_info.cpp matrix_operations.cpp -static
```

### Execution
```bash
./matrix_multiply
```

## Algorithm Deep Dive: Strassen's Method

### The Mathematical Foundation

Strassen's algorithm, developed by Volker Strassen in 1969, revolutionized matrix multiplication by reducing the number of scalar multiplications required. While traditional matrix multiplication requires 8 multiplications for 2×2 matrices, Strassen's method accomplishes the same result with only 7 multiplications.

### How Strassen's Algorithm Works

#### Matrix Decomposition
For two n×n matrices A and B, the algorithm divides each matrix into four (n/2)×(n/2) submatrices:

```
A = [A11  A12]    B = [B11  B12]
    [A21  A22]        [B21  B22]
```

#### The Seven Products
Instead of computing 8 products directly, Strassen defines 7 intermediate products:

- **P1** = A11(B12 - B22)
- **P2** = (A11 + A12)B22
- **P3** = (A21 + A22)B11
- **P4** = A22(B21 - B11)
- **P5** = (A11 + A22)(B11 + B22)
- **P6** = (A12 - A22)(B21 + B22)
- **P7** = (A11 - A21)(B11 + B12)

#### Result Assembly
The final result matrix C is constructed as:
- **C11** = P5 + P4 - P2 + P6
- **C12** = P1 + P2
- **C21** = P3 + P4
- **C22** = P5 + P1 - P3 - P7

### Optimization Strategies in Fluminum

#### Threshold-Based Switching
The implementation uses an intelligent threshold system (`STRASSEN_THRESHOLD = 32`) to determine when to switch from Strassen's recursive approach to naive multiplication. This prevents the overhead of recursion from negating benefits on small matrices.

#### Memory Layout Optimization
Submatrix operations are optimized for cache efficiency, ensuring that data access patterns align with CPU cache hierarchies. This significantly reduces memory latency and improves overall performance.

#### Parallelization of Recursive Calls
The seven products (P1-P7) are computed in parallel across available CPU cores, with careful synchronization to maintain data integrity while maximizing throughput.

## Architecture and Implementation

### File Structure and Components

#### `system_info.h` / `system_info.cpp`
**Purpose**: Hardware detection and system adaptation

**Key Functions**:
- `getCpuInfo()`: Retrieves CPU model, core count, and threading capabilities
- `calculateOptimalThreads()`: Determines optimal thread count based on matrix size and hardware
- Cross-platform detection for Windows (WinAPI) and Linux (procfs parsing)

**Variables**:
- `cpuName`: String containing CPU model identification
- `cpuCores`: Physical core count
- `cpuThreads`: Logical thread count (including hyperthreading)
- `reservedThreadsForOS`: Threads reserved for system operations (default: 2)

#### `matrix_operations.h` / `matrix_operations.cpp`
**Purpose**: Core matrix computation algorithms

**Key Functions**:
- `multiplyMatricesStrassen()`: Main Strassen algorithm implementation
- `multiplyMatricesNaive()`: Traditional O(n³) multiplication for small matrices
- `addMatrices()` / `subtractMatrices()`: Matrix arithmetic operations
- `getSubMatrix()` / `combineSubMatrices()`: Submatrix manipulation utilities
- `multiplicationWorker()`: Thread worker function for parallel computation

**Algorithm Flow**:
1. Size check against threshold
2. Matrix subdivision into quadrants
3. Parallel computation of seven products
4. Result assembly and combination

#### `Flens.cpp`
**Purpose**: Main program orchestration and testing framework

**Core Logic**:
- System analysis and thread calculation
- Matrix initialization and memory allocation
- Performance timing and measurement
- Scalability testing with increasing matrix sizes
- Results validation and output formatting

### Threading Architecture

#### Thread Pool Management
The implementation uses a sophisticated thread pool system that:
- Pre-allocates worker threads based on system capabilities
- Distributes Strassen's seven products across available cores
- Implements work-stealing for load balancing
- Uses condition variables for efficient thread synchronization

#### Synchronization Mechanisms
- **Mutexes**: Protect shared data structures during matrix operations
- **Condition Variables**: Coordinate thread execution and completion
- **Atomic Operations**: Ensure thread-safe counter and flag operations
- **Memory Barriers**: Maintain memory ordering consistency

## Performance Analysis: Revolutionary Results

### Testing Methodology

Performance testing was conducted across diverse hardware configurations using standardized 1024×1024 matrices filled with constant values. Each test measured:
- **Naive Multiplication Time (OM_Time)**: Traditional O(n³) approach
- **Strassen Single-Thread Time**: Strassen algorithm on single core
- **Strassen Multi-Thread Time (SA_Time)**: Fully parallelized implementation
- **Speedup Ratios**: Comparative performance improvements

### Hardware Test Results

#### Intel Core i9-13900K (24 cores, 32 threads)
- **Naive Method**: 178.498 seconds
- **Strassen (1 thread)**: 7.6108 seconds → **23.4x speedup**
- **Strassen (32 threads)**: 0.3110 seconds → **574x total speedup**
- **Parallel Efficiency**: 24.5x improvement from threading alone

#### Intel Core i5-12400 (6 cores, 12 threads)
- **Naive Method**: 220.85 seconds  
- **Strassen (1 thread)**: 11.84313 seconds → **18.6x speedup**
- **Strassen (12 threads)**: 1.83452 seconds → **120x total speedup**
- **Parallel Efficiency**: 6.45x improvement from threading

#### Intel Core i7-8600U (4 cores, 8 threads)
- **Naive Method**: 712.19 seconds
- **Strassen (1 thread)**: 15.4439 seconds → **46x speedup**
- **Strassen (8 threads)**: 4.3633 seconds → **163x total speedup**
- **Parallel Efficiency**: 3.54x improvement from threading

#### AMD Ryzen 5 7535HS (6 cores, 12 threads)
- **Naive Method**: 335.483 seconds
- **Strassen (1 thread)**: 13.1583 seconds → **25.5x speedup**
- **Strassen (12 threads)**: 2.31333 seconds → **145x total speedup**
- **Parallel Efficiency**: 5.69x improvement from threading

#### AMD Ryzen 5 7530U (6 cores, 12 threads)
- **Naive Method**: 252.505 seconds
- **Strassen (1 thread)**: 12.999141 seconds → **19.4x speedup**
- **Strassen (12 threads)**: 2.502371 seconds → **100x total speedup**
- **Parallel Efficiency**: 5.2x improvement from threading

### Performance Analysis Insights

#### Algorithmic Superiority
The transition from O(n³) to O(n^2.807) complexity provides substantial benefits that become more pronounced with larger matrices. Even on older hardware (i7-8600U), the algorithmic improvement alone delivers 46x speedup.

#### Parallelization Effectiveness
Multi-threading provides additional performance multiplication on top of algorithmic improvements:
- **High-end processors** (i9-13900K): Up to 24.5x additional speedup
- **Mid-range processors** (i5-12400, Ryzen 5): 5-6x additional speedup
- **Lower-end processors** (i7-8600U): 3-4x additional speedup

#### Hardware Correlation Factors

**Core Count Impact**: Processors with more cores show better parallel scaling, with diminishing returns beyond optimal thread counts.

**Architecture Efficiency**: Modern architectures (12th/13th gen Intel, Ryzen 7000 series) demonstrate superior single-thread performance and better parallel scaling.

**Cache Hierarchy Benefits**: The block-based nature of Strassen's algorithm improves cache locality, contributing to performance gains beyond algorithmic complexity reduction.

## Usage Instructions

### Basic Execution
```bash
./matrix_multiply
```

### Expected Output
```
Starting Thread-Matrix Enhanced with Strassen...
Analyzing system...
CPU Name: Intel(R) Core(TM) i9-13900K
CPU Cores: 24
CPU Threads (Logical): 32

Generator: Creating matrices of size 1024x1024
Number of worker threads for this round: 32
Timer: [Real-time updates during computation]
Timer finished: 0.3110 seconds

Calculation complete for matrices of size 1024x1024 using Strassen!
Number of worker threads (based on matrix size): 32
Input elements: 2097152, Output elements: 1048576
Total Computation time: 0.3110 seconds

Speedup over naive method: 574.11x
Parallel efficiency: 24.47x
```

### Understanding the Output

- **System Analysis**: CPU detection and capability assessment
- **Matrix Generation**: Memory allocation and initialization
- **Thread Calculation**: Optimal thread count determination
- **Real-time Timer**: Live computation progress
- **Performance Metrics**: Comprehensive speed and efficiency analysis

### Customization Options

#### Matrix Size Modification
```cpp
// In Flens.cpp, modify the starting size
int matrixSize = 128; // Start with 128x128 instead of 64x64
```

#### Thread Count Override
```cpp
// Force specific thread count
int forceThreads = 16; // Override automatic calculation
```

#### Threshold Adjustment
```cpp
// In matrix_operations.cpp
const int STRASSEN_THRESHOLD = 64; // Increase threshold for different behavior
```

## Technical Optimizations

### Cache-Friendly Implementation
- **Block-wise Operations**: Submatrix operations maintain spatial locality
- **Memory Alignment**: Data structures aligned for optimal cache line usage
- **Prefetching**: Strategic memory prefetching for predictable access patterns

### Numerical Stability
- **Precision Maintenance**: Careful handling of floating-point arithmetic
- **Error Accumulation Control**: Monitoring and mitigation of computational errors
- **Overflow Protection**: Safe handling of large matrix values

### Memory Management
- **Dynamic Allocation**: Efficient memory allocation for large matrices
- **Memory Pool**: Reuse of allocated memory blocks to reduce allocation overhead
- **RAII Principles**: Automatic resource management and cleanup

## Future Development Roadmap

### Immediate Enhancements
- **GPU Acceleration**: CUDA and OpenCL implementations for massive parallelization
- **Memory Optimization**: Further cache-friendly optimizations and memory layout improvements
- **Precision Options**: Support for different numerical precisions (float, double, long double)

### Advanced Features
- **Distributed Computing**: MPI implementation for cluster-based computation
- **Dynamic Load Balancing**: Runtime workload redistribution
- **Adaptive Algorithms**: Machine learning-based parameter optimization

### Platform Extensions
- **Mobile Optimization**: ARM architecture support and mobile-specific optimizations
- **Web Assembly**: Browser-based matrix multiplication
- **Embedded Systems**: Microcontroller and IoT device adaptations

## Contributing Guidelines

### Code Standards
- Follow C++11/14 standards with modern best practices
- Maintain comprehensive inline documentation
- Include unit tests for all new functionality
- Ensure cross-platform compatibility

### Performance Testing
- Benchmark against multiple hardware configurations
- Document performance characteristics and limitations
- Provide before/after performance comparisons

### Pull Request Process
1. Fork the repository and create feature branches
2. Implement changes with appropriate testing
3. Document new features and modifications
4. Submit pull requests with detailed descriptions

## Conclusion

Fluminum represents a paradigm shift in high-performance matrix computation, demonstrating that theoretical algorithmic improvements can be translated into dramatic real-world performance gains. The combination of Strassen's algorithm with sophisticated parallelization delivers speedups ranging from 100x to over 500x compared to traditional approaches.

The project showcases the importance of:
- **Algorithmic Innovation**: Moving beyond brute-force approaches
- **Hardware Utilization**: Leveraging modern multi-core architectures
- **Software Engineering**: Clean, maintainable, and extensible code design

This implementation serves as both a practical tool for large-scale matrix operations and a educational example of advanced optimization techniques in computational mathematics.

## License

This project is licensed under the MIT License - see the LICENSE file for details.

## Acknowledgments

- Volker Strassen for the revolutionary matrix multiplication algorithm
- The C++ community for excellent threading and system libraries
- Hardware manufacturers (Intel, AMD) for detailed processor documentation
- Open source contributors and testers across various platforms
