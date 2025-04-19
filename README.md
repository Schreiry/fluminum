An efficient algorithm for matrix operations with Strassen algorithm (EAMO)
====================================

Project Overview
----------------

This project implements an efficient matrix multiplication system using **Strassen's algorithm**, a divide-and-conquer approach that reduces the time complexity of matrix multiplication from O(n³) to approximately O(n².⁸⁰⁷). Written in C++, the project includes a framework for multi-threading to leverage modern multi-core CPUs, although the current implementation performs the Strassen multiplication in a single thread. The code adapts to different hardware by gathering CPU information and calculating an optimal number of threads based on matrix size and system capabilities.

The project demonstrates significant performance improvements over traditional naive matrix multiplication, with reduced computation times and lower CPU heating, making it an efficient solution for large-scale matrix operations.

Features
--------

*   **Strassen's Algorithm**: Optimized matrix multiplication with a complexity of O(n².⁸⁰⁷).
    
*   **System Information Detection**: Gathers CPU details (name, cores, threads) to optimize thread usage across Windows and Linux.
    
*   **Multi-Threading Framework**: Includes thread management with std::thread, std::mutex, and std::condition\_variable, with potential for future parallelization.
    
*   **Performance Timer**: Displays real-time computation duration for each multiplication round.
    
*   **Scalability Testing**: Automatically increases matrix sizes (starting at 64x64, doubling each round) to evaluate performance.
    

Requirements
------------

*   **C++ Compiler**: Must support C++11 or later (e.g., g++).
    
*   **Operating System**: Compatible with Windows or Linux.
    
*   **Hardware**: Multi-core CPU recommended for optimal performance (tested on Intel i9-13900K).
    

Installation
------------

1.  git clone https://github.com/your-repo/thread-matrix-strassen.git
    
2.  cd thread-matrix-strassen
    
3.  g++ -std=c++11 -o matrix\_multiply Flens.cpp system\_info.cpp matrix\_operations.cpp -lpthread
    
    *   On Windows, you may need to adjust the command depending on your compiler (e.g., MinGW or MSVC).
        

Usage
-----

1.  ./matrix\_multiply
    
2.  **What to Expect**:
    
    *   The program starts by analyzing your system's CPU (name, cores, threads).
        
    *   It generates two matrices (A and B) of size 64x64, filled with constant values (1 and 2, respectively), and a result matrix C initialized to zero.
        
    *   It performs matrix multiplication using Strassen's algorithm, displaying:
        
        *   Matrix size (e.g., 64x64).
            
        *   Number of worker threads (calculated but not yet utilized for computation).
            
        *   Real-time timer updates.
            
        *   Total computation time per round.
            
        *   Input and output element counts.
            
    *   The matrix size doubles each round (64, 128, 256, ..., up to system limits), with a 2-second delay between rounds.
        
3.  Starting Thread-Matrix Enhanced with Strassen...Analyzing system...CPU Name: Intel(R) Core(TM) i9-13900KCPU Cores: 24CPU Threads (Logical): 32Generator: Creating matrices of size 1024x1024Number of worker threads for this round: 8Timer finished: 11.523 secondsCalculation complete for matrices of size 1024x1024 using Strassen!Number of worker threads (based on matrix size): 8Input elements: 2097152, Output elements: 1048576Total Computation time: 11.523 seconds
    

Implementation Details
----------------------

### 1\. System Information Gathering

*   **Files**: system\_info.h, system\_info.cpp
    
*   **Purpose**: Retrieves CPU details to adapt the program to the hardware.
    
*   **How It Works**:
    
    *   **Windows**: Uses GetSystemInfo for core/thread counts and registry queries for CPU name.
        
    *   **Linux**: Parses /proc/cpuinfo and uses sysconf for core/thread counts.
        
*   **Key Variables**:
    
    *   cpuName: CPU model (e.g., "Intel(R) Core(TM) i9-13900K").
        
    *   cpuCores: Physical cores.
        
    *   cpuThreads: Logical threads (hyper-threading).
        
    *   reservedThreadsForOS: Reserves 2 threads for the operating system.
        

### 2\. Matrix Operations

*   **Files**: matrix\_operations.h, matrix\_operations.cpp
    
*   **Functions**:
    
    *   addMatrices / subtractMatrices: Helper functions for Strassen's submatrix operations.
        
    *   multiplyMatricesNaive: O(n³) multiplication for small matrices (below STRASSEN\_THRESHOLD = 32).
        
    *   multiplyMatricesStrassen: Implements Strassen's algorithm:
        
        *   Divides matrices into four submatrices (A11, A12, A21, A22, etc.).
            
        *   Performs seven recursive multiplications (P1-P7).
            
        *   Combines results into the final matrix C.
            
    *   getSubMatrix / combineSubMatrices: Utility functions for submatrix manipulation.
        
*   **Threshold**: Switches to naive multiplication for matrices smaller than 32x32 to avoid Strassen's overhead.
    

### 3\. Main Program

*   **File**: Flens.cpp (intended as main.cpp)
    
*   **Logic**:
    
    *   Starts with a 64x64 matrix, ensuring power-of-two sizes for Strassen compatibility.
        
    *   Initializes matrices A (all 1s), B (all 2s), and C (all 0s).
        
    *   Performs Strassen multiplication in the main thread.
        
    *   Uses a timer thread to display elapsed time.
        
    *   Doubles matrix size each iteration for scalability testing.
        

### 4\. Thread Management

*   **Current State**: Multi-threading is partially implemented:
    
    *   multiplicationWorker: Defined but only prints messages; no computation yet.
        
    *   calculateOptimalThreads: Determines thread count based on matrix size (logarithmic scaling, clamped between 1 and available threads minus 2).
        
*   **Synchronization**: Uses std::mutex, std::condition\_variable, and std::atomic for thread safety.
    

Performance
-----------
```markdown
### Observed Results

*   Hardware: Intel i9-13900K (24 cores, 32 threads).
    
*   example : atrix Size: 1024x1024.
    
*   Execution Time :
    
    *   This Implementation : 11.1–11.9 seconds.
        
    *   Naive Mode : ~14.6 seconds.
        
*   Speedup : Approximately 20–24% faster than naive multiplication.
    
*   CPU Usage : Does not reach 100%, reducing CPU heating while maintaining high efficiency.
```

### Why It’s Faster

*   **Strassen’s Algorithm**: Reduces the number of multiplications from 8 to 7 per recursive step, lowering the total operation count.
    
*   **Single-Threaded Efficiency**: Current implementation uses one core effectively with Strassen’s optimization, avoiding threading overhead for now.
    

### Reduced CPU Heating

*   Since the computation is single-threaded, only one core is heavily utilized, leaving others idle. This contrasts with a fully parallel naive approach that might max out all cores, increasing heat.
    
*   Strassen’s fewer operations also mean less overall CPU work, contributing to lower thermal output.
    

Quality Assessment
------------------

*   **Strengths**:
    
    *   Clean, modular code structure with separate headers for system info and matrix operations.
        
    *   Adaptive to different hardware via CPU detection.
        
    *   Significant performance gain for large matrices due to Strassen’s algorithm.
        
*   **Limitations**:
    
    *   Multi-threading is not fully utilized; Strassen multiplication runs in the main thread.
        
    *   Limited to power-of-two matrix sizes (padding not yet implemented for others).
        
    *   Threshold (32) may need tuning for optimal performance across hardware.
        

Future Improvements
-------------------

*   **Parallelize Strassen’s Algorithm**: Distribute the seven recursive multiplications (P1-P7) across threads to utilize multiple cores.
    
*   **Dynamic Threshold**: Adjust STRASSEN\_THRESHOLD based on hardware and matrix size for optimal switching between naive and Strassen methods.
    
*   **Support Non-Power-of-Two Sizes**: Implement padding or alternative techniques.
    
*   **Enhanced Threading**: Fully integrate multiplicationWorker to handle submatrix computations.
    

Contributing
------------

Contributions are encouraged! Potential areas include:

*   Parallelizing Strassen’s algorithm.
    
*   Optimizing performance for specific hardware.
    
*   Adding support for non-square or non-power-of-two matrices.
    

Submit pull requests or open issues on the GitHub repository.

License
-------

This project is licensed under the **MIT License**.
