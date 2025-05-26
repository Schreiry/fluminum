# Fluminum (v2.2): The Apex of High-Performance Matrix Operations in C++ 🚀
   ![Intel](https://a11ybadges.com/badge?logo=intel) ![AMD](https://a11ybadges.com/badge?logo=amd) ![Visual Studio](https://img.shields.io/badge/Visual%20Studio-5C2D91.svg?style=for-the-badge&logo=visual-studio&logoColor=white) ![C++](https://img.shields.io/badge/c++-%2300599C.svg?style=for-the-badge&logo=c%2B%2B&logoColor=white) ![Windows](https://img.shields.io/badge/Windows-0078D6?style=for-the-badge&logo=windows&logoColor=white)  ![Windows 11](https://img.shields.io/badge/Windows%2011-%230079d5.svg?style=for-the-badge&logo=Windows%2011&logoColor=white) 	![GitHub](https://img.shields.io/badge/github-%23121011.svg?style=for-the-badge&logo=github&logoColor=white)
 
Welcome to **Fluminum**, an interactive C++ powerhouse engineered to shatter the boundaries of matrix computation speed. This project isn't just another implementation; it's a testament to the synergistic power of **Strassen's algorithm**, advanced **multi-threading with `std::async`**, low-level **SIMD (AVX/SSE2) optimizations**, and intelligent **system-aware resource management**.

Forget the days of sluggish $O(N^3)$ operations. Fluminum delivers **astonishing performance gains, achieving speedups of up to 574x** compared to traditional methods on modern hardware. It features a user-friendly interactive console interface, real-time progress bars, detailed performance analytics, and robust error handling, making it both a powerful tool and an exceptional learning platform.

[](https://www.google.com/search?q=https://github.com/Schreiry/fluminum/blob/main/LICENSE)
[](https://isocpp.org/)
[](https://www.microsoft.com/windows/)


## 🔠 Language variability for documentation : 
 > [!TIP]
 > 
 > [ ქართულ ენაზე ](https://github.com/Schreiry/fluminum/blob/main/README%5B%20%E1%83%A5%E1%83%90%20%5D.md)
 >
 >
 > [На русском языке](https://www.microsoft.com/windows/)


## 📚 Documentation

- [📖 User Guide](docs/user-guide.md)
- [🏗️ Architecture Overview](docs/architecture.md)
- [⚡ Performance Tuning](docs/performance.md)



## ✨ Key Features

  * **⚡ Blazing Speed:** Combines Strassen's $O(N^{2.807})$ complexity with deep parallelism and SIMD for unparalleled performance.
  * **🧠 Intelligent Strassen:** Implements a **recursive, parallel Strassen's algorithm** with an **adaptive threshold** system, switching to SIMD-optimized naive multiplication for smaller sub-problems to maximize efficiency.
  * **🌐 Multi-Core Mastery:** Leverages `std::async` and `std::future` to **dynamically distribute the 7 recursive Strassen calls** and comparison tasks across all available CPU cores.
  * **🏎️ SIMD Acceleration:** The base-case (naive) multiplication is supercharged with **AVX and SSE2 intrinsics**, processing multiple `double` values per clock cycle where hardware allows.
  * **🖥️ System-Aware:** Automatically **detects CPU cores, available memory, and SIMD support** (AVX/SSE2), providing memory estimates and warnings.
  * **📊 Interactive & Informative:** Features a **rich console UI** with colors, boxes, an animated progress bar, sound notifications, and detailed performance breakdowns (including QPC and Chrono timers) and **CSV logging**.
![Screenshot 2025-05-23 203237](https://github.com/user-attachments/assets/f2193ee0-f37d-4904-b2a1-d4bbb493eb83)
![Screenshot 2025-05-23 212430](https://github.com/user-attachments/assets/51d48adf-5aee-494a-bb34-3e5e64c17c84)


  * **🛠️ Robust & Flexible:** Handles **non-power-of-two matrices** through automatic padding/unpadding, provides various input methods (random, console, file), and allows saving results.
  * **🔍 Parallel Comparison:** Offers a recursive, parallel matrix comparison function with **epsilon support** for floating-point accuracy checks.
  * **📝 Modern C++:** Built with C++17, emphasizing clean code, RAII, and modern concurrency features.

-----

## 💡 Strassen's Algorithm: The Divide-and-Conquer Revolution

Traditional matrix multiplication is a straightforward, yet computationally intensive, process with a cubic complexity ($O(N^3)$). This means doubling the matrix size increases the workload eightfold, quickly becoming impractical for large matrices.

Volker Strassen's 1969 algorithm changed the game. It applies a "divide and conquer" strategy:

1.  **Divide:** A large $N \\times N$ matrix multiplication is broken down into operations on smaller $N/2 \\times N/2$ sub-matrices.
2.  **Conquer (The Trick):** Instead of the 8 sub-matrix multiplications required by the naive approach, Strassen cleverly rearranges the calculations to require only **7 multiplications**. This is achieved by introducing 10 intermediate sum/difference matrices (S1-S10) and calculating 7 product matrices (P1-P7).
3.  **Combine:** The 7 product matrices are combined (through addition and subtraction) to form the four quadrants of the final result matrix.

$$A = \begin{bmatrix} A_{11} & A_{12} \\ A_{21} & A_{22} \end{bmatrix}, \quad B = \begin{bmatrix} B_{11} & B_{12} \\ B_{21} & B_{22} \end{bmatrix}, \quad C = \begin{bmatrix} C_{11} & C_{12} \\ C_{21} & C_{22} \end{bmatrix}$$

  * $S\_1 = B\_{12} - B\_{22}$
  * $S\_2 = A\_{11} + A\_{12}$
  * $S\_3 = A\_{21} + A\_{22}$
  * $S\_4 = B\_{21} - B\_{11}$
  * $S\_5 = A\_{11} + A\_{22}$
  * $S\_6 = B\_{11} + B\_{22}$
  * $S\_7 = A\_{12} - A\_{22}$
  * $S\_8 = B\_{21} + B\_{22}$
  * $S\_9 = A\_{11} - A\_{21}$
  * $S\_{10} = B\_{11} + B\_{12}$
  * $P\_1 = S\_5 \\times S\_6$
  * $P\_2 = S\_3 \\times B\_{11}$
  * $P\_3 = A\_{11} \\times S\_1$
  * $P\_4 = A\_{22} \\times S\_4$
  * $P\_5 = S\_2 \\times B\_{22}$
  * $P\_6 = S\_9 \\times S\_{10}$
  * $P\_7 = S\_7 \\times S\_8$
  * $C\_{11} = P\_1 + P\_4 - P\_5 + P\_7$
  * $C\_{12} = P\_3 + P\_5$
  * $C\_{21} = P\_2 + P\_4$
  * $C\_{22} = P\_1 - P\_2 + P\_3 + P\_6$

This recursive reduction from 8 to 7 multiplications leads to the $O(N^{2.807})$ complexity. While the number of additions/subtractions increases, for large N, the reduction in multiplications dominates, leading to significant speedups. Fluminum manages the overhead by **switching to an SIMD-optimized naive method below a configurable threshold**, ensuring peak performance across all scales.

-----

## ⚙️ The Fluminum Edge: Deep Optimizations

Fluminum achieves its performance through a multi-pronged optimization strategy:

1.  **Recursive Parallelism (`std::async`)**: The `strassen_recursive_worker` function lies at the heart. When a matrix is split, the 7 subsequent recursive calls (P1-P7) are launched as **asynchronous tasks using `std::async`**. This creates a tree of parallel computations. The depth of this parallelism (`max_depth_async`) is calculated based on the available hardware threads, ensuring optimal core utilization without excessive thread creation overhead.
2.  **SIMD-Accelerated Base Case (`multiply_naive`)**: Strassen's recursion eventually hits a base case. Instead of a simple naive loop, Fluminum's `multiply_naive` leverages **Single Instruction, Multiple Data (SIMD) intrinsics**. It checks for **AVX** support first, falling back to **SSE2** if AVX isn't available. This allows the CPU to perform multiplications on 4 (AVX) or 2 (SSE2) `double` values simultaneously, drastically speeding up the most numerous, small-scale multiplications.
3.  **Adaptive Thresholding**: The user can specify a `threshold`. Matrices smaller than this size are processed by the fast `multiply_naive` function, avoiding the overhead of Strassen's recursion and matrix additions/subtractions where they aren't beneficial. Setting the threshold to 0 forces naive (but still SIMD-accelerated) multiplication.
4.  **Automatic Padding**: Strassen's algorithm (as implemented here) requires square matrices with dimensions that are powers of two. Fluminum **automatically handles any input size** by padding the matrices with zeros up to the next power of two (`Matrix::pad`) before computation and then trimming them back to the original size (`Matrix::unpad`) afterwards.
5.  **Memory & System Awareness**: Before starting, the program checks available RAM and estimates the peak memory usage, issuing warnings if necessary. It uses `GlobalMemoryStatusEx` and `GetProcessMemoryInfo` (Windows-specific) for this.
6.  **User Experience Focus**: A `display_progress` function runs in a separate thread, using an `std::atomic<int>` counter to provide a **real-time progress bar** during long computations. The console output is enhanced with colors and structured boxes for better readability.

-----

## 🏗️ Architecture Overview

```
┌─────────────────────────────────────────────────────────────┐
│                    Fluminum Architecture                    │
├─────────────────────────────────────────────────────────────┤
│  Interactive Console UI  │  Progress Tracking  │  Logging   │
├─────────────────────────────────────────────────────────────┤
│           Strassen Algorithm (O(N^2.807))                   │
│  ┌─────────────────┐  ┌─────────────────┐  ┌──────────────┐ │
│  │  Divide Matrix  │  │ Parallel Tasks  │  │ Combine      │ │
│  │   (std::async)  │  │   (7 Products)  │  │ Results      │ │
│  └─────────────────┘  └─────────────────┘  └──────────────┘ │
├─────────────────────────────────────────────────────────────┤
│              SIMD-Optimized Base Case                       │
│  ┌─────────────┐  ┌─────────────┐  ┌─────────────────────┐  │
│  │     AVX     │  │    SSE2     │  │   Fallback Naive    │  │
│  │  (4 doubles)│  │ (2 doubles) │  │    (1 double)       │  │
│  └─────────────┘  └─────────────┘  └─────────────────────┘  │
├─────────────────────────────────────────────────────────────┤
│                   System Integration                        │
│  Memory Management  │  Hardware Detection  │  Error Handling│
└─────────────────────────────────────────────────────────────┘
```



The single C++ file `fluminumTversion.cpp` is a self-contained unit demonstrating these principles.

  * **Includes**: A comprehensive set of standard library headers (`<vector>`, `<thread>`, `<future>`, `<atomic>`, `<chrono>`, `<immintrin.h>`) and Windows-specific headers (`<windows.h>`, `<psapi.h>`) are used.
  * **Console Formatting**: A set of functions (`print_header_box`, `print_line_in_box`) and constants handle the colored, structured console output.
  * **System Info**: Functions like `getSystemMemoryInfo`, `getCpuCoreCount`, `check_simd_support`, and `estimateStrassenMemoryMB` provide hardware insights.
  * **`Matrix` Class**:
      * **Data**: Uses `std::vector<double>` for flat, contiguous storage.
      * **Constructors**: Allows creation from dimensions, initial values, or `std::vector<std::vector<double>>`.
      * **Operators**: `+` and `-` for sub-matrix operations.
      * **`multiply_naive`**: The SIMD-optimized base case.
      * **`split` / `combine`**: Handles the divide-and-conquer mechanics.
      * **`pad` / `unpad`**: Ensures power-of-two dimensions.
      * **I/O**: `readFromConsole`, `readFromFile`, `saveToFile`.
  * **Strassen Implementation**:
      * **`multiplyStrassenParallel`**: The main entry point. Handles padding, thread calculation, progress bar setup, and calls the worker.
      * **`strassen_recursive_worker`**: The core recursive function. Implements the 7-multiplication logic and launches tasks via `std::async`.
  * **Comparison Implementation**:
      * **`compareMatricesParallel`**: Similar structure, recursively divides matrices and compares quadrants in parallel.
      * **`compareMatricesInternal`**: The recursive worker for comparison.
  * **Main Loop (`main`, `run_one_operation`)**: Drives the interactive user interface, handles input validation, orchestrates operations, displays results, and manages logging.

-----

## 📊 Performance Benchmarks: A Leap in Efficiency

> [!NOTE]
> The true power of Fluminum is evident in its benchmark results. The following table shows the execution times for multiplying two **2048x2048** matrices on various CPUs, comparing naive $O(N^3)$ (OM\_Time) with Fluminum's > parallel Strassen (SA\_Time).

> [!IMPORTANT]
>
> | Processor | L3 Cache | Cores/Threads | Base Clock | Turbo Clock | Memory | Memory Capacity |
> |---|---|---|---|---|---|---|
> | Intel i9-13900K | 36 MB Intel® Smart Cache | 24/32 | 3.0 GHz | 5.9 GHz | DDR5-5600 | 128GB |
> | Intel i5-12400 | 18 MB Intel® Smart Cache | 6/12 | 2.4 GHz | 4.4 GHz | DDR5-5200 | 32GB |
> | Intel i7-8600U | 8 MB Intel® Smart Cache | 4/8 | 1.90 GHz | 4.20 GHz | DDR4-3200 | 16GB |
> | AMD Ryzen 5 7535HS | 16 MB | 6/12 | 3.3 GHz | 4.55 GHz | DDR5-4800 | 16GB |
> | AMD Ryzen 5 7530U | 16 MB | 6/12 | 2 GHz | 4.4 GHz | DDR4-3600 | 16GB |
> 
> 
> 
> 
> | Processor | Threads | Naive (OM\_Time, s) | Strassen (SA\_Time, s) | **Total Speedup (OM / SA)** | Parallel Speedup (SA\_1 / SA\_N) |
> | :--- | :--- | :--- | :--- | :--- | :--- |
> | **Intel Core i9-13900K** | 1 | 178.498 | 7.6108 | **\~23.4x** | 1.0x |
> | | **32** | 178.498 | **0.3110** | **\~574.0x** | **\~24.5x** |
> | **Intel Core i5-12400** | 1 | 220.850 | 11.8431 | **\~18.6x** | 1.0x |
> | | **12** | 220.850 | **1.8345** | **\~120.4x** | **\~6.5x** |
> | **Intel Core i7-8600U** | 1 | 712.190 | 15.4439 | **\~46.1x** | 1.0x |
> | | **8** | 712.190 | **4.3633** | **\~163.2x** | **\~3.5x** |
> | **AMD Ryzen 5 7535HS** | 1 | 335.483 | 13.1583 | **\~25.5x** | 1.0x |
> | | **12** | 335.483 | **2.3133** | **\~145.0x** | **\~5.7x** |
> | **AMD Ryzen 5 7530U** | 1 | 252.505 | 12.9991 | **\~19.4x** | 1.0x |
> | | **12** | 252.505 | **2.5024** | **\~100.9x** | **\~5.2x** |

> [!TIP]
> | CPU | Single Thread | Multi-Thread | **Total Speedup** |
> |-----|---------------|--------------|-------------------|
> | **i9-13900K** | 23.4× | **574.0×** | 🏆 **Champion** |
> | **i5-12400** | 18.6× | **120.4×** | 🥈 **Excellent** |
> | **Ryzen 5 7535HS** | 25.5× | **145.0×** | 🥉 **Outstanding** |

**Observations:**

  * **Algorithmic Dominance:** Strassen alone provides a 18x to 46x speedup.
  * **Parallel Power:** Multi-threading adds another 3.5x to 24.5x boost.
  * **Synergistic Explosion:** The combined effect yields incredible 100x to 574x improvements, turning minute-long calculations into sub-second tasks.
  * **Scalability:** The performance scales well with core count, demonstrating efficient parallelization.
  * **SIMD Impact:** The already impressive gains are further amplified by the SIMD-optimized base case, though its individual contribution isn't isolated in these high-level tests.

-----

## 🔬 Technical Deep Dive

### Memory Layout Optimization

```cpp
class Matrix {
    std::vector<double> data;  // Contiguous memory layout
    size_t rows, cols;         // Dimensions
    
    // Cache-friendly access patterns
    inline double& operator()(size_t i, size_t j) {
        return data[i * cols + j];
    }
};
```

### SIMD Implementation

```cpp
void multiply_naive_avx(const Matrix& A, const Matrix& B, Matrix& C) {
    for (size_t i = 0; i < A.rows; ++i) {
        for (size_t j = 0; j < B.cols; j += 4) {
            __m256d sum = _mm256_setzero_pd();
            for (size_t k = 0; k < A.cols; ++k) {
                __m256d a = _mm256_broadcast_sd(&A(i, k));
                __m256d b = _mm256_loadu_pd(&B(k, j));
                sum = _mm256_fmadd_pd(a, b, sum);
            }
            _mm256_storeu_pd(&C(i, j), sum);
        }
    }
}
```




## 🛠️ Requirements & Setup

  * **Operating System:** Windows (heavily utilizes WinAPI for console features and system info). Adaptation to Linux/macOS is possible but requires replacing Windows-specific calls.
  * **Compiler:** C++17 support required. **Microsoft Visual C++ (MSVC)** is recommended due to `_MSC_VER` checks, intrinsics, and `#pragma comment(lib, "Psapi.lib")`. (Can be adapted for GCC/Clang with minor changes).
  * **Hardware:** A multi-core CPU is highly recommended to benefit from parallelism. AVX or SSE2 support is needed for full SIMD acceleration.

### Installation & Compilation

1.  **Clone the Repository:**

    ```bash
    git clone https://github.com/Schreiry/fluminum.git
    cd fluminum
    ```

2.  **Compile with MSVC (Recommended):**

      * Open the "Developer Command Prompt for VS".
      * Navigate to the `fluminum` directory.
      * Compile using:
        ```bash
        cl fluminumTversion.cpp /EHsc /O2 /std:c++17 /Fe:fluminum.exe /link Psapi.lib
        ```
          * `/O2`: Enables speed optimizations.
          * `/std:c++17`: Sets the C++ standard.
          * `/link Psapi.lib`: Links the Process Status API library (for memory info).

### Usage

Run the compiled executable:

```bash
./fluminum.exe
```

The program will greet you with an interactive menu:

1.  **System Information:** Displays detected RAM, CPU cores, and SIMD support.
2.  **Select Operation:** Choose between Matrix Multiplication or Matrix Comparison.
3.  **Logging:** Opt to log performance results to a CSV file.
4.  **Dimensions:** Enter the matrix sizes.
5.  **Memory Estimation:** See an estimate of the required RAM.
6.  **Input Method:** Choose random generation, console input, or file input.
7.  **Settings:** Configure the Strassen threshold, number of threads, and (for comparison) epsilon.
8.  **Execution:** Watch the progress bar as the calculation runs.
9.  **Results:** View detailed performance statistics and timings.
10. **Save & Continue:** Optionally save the result matrix and run another operation.

-----


## 📈 Roadmap

### Version 2.2 + (Q2 2025) Now

- [X] SIMD/AVX2 (256) support ;

- [x] acceptable UI ;

- [x] multiple efficiency on the processor

- [x] optimization. compared to previous versions, it is excellent.
       memory is used more efficiently, smarter and more rationally. when multiplying matrices: 2048X 2048 X 2048X 2048, the program used 1654 ~ 1850 MB, now it uses 800 ~ 990 MB . 

- [x] Strassen's algorithm ; 

- [x] Thresholds ;

- [x] less CPU heating ;

- [ ] coefficient system

- [ ] student conference ;




### Version 2.5 - 2.9 (Q3 2025)

- [ ] Cross-platform support (Linux/macOS) ; 
 
- [ ] AVX-512 support ; 
 
- [ ] Better UI ; 
 
- [ ] Profiling and testing ; 
 
- [ ] Dynamic threshold optimization 

### Version 3.0 (Q4 2025)

- [ ] Distributed computing (MPI) ;

- [ ] open library ;
 
- [ ] Python bindings ; 
 
- [ ] Web assembly port ; 
 
- [ ] GPU acceleration (CUDA/OpenCL) ; 
 
- [ ] Quantum-resistant algorithms




-----

## 🤝 Contributing

Contributions are highly welcome\! If you have ideas for improvements, bug fixes, or new features:

1.  **Fork** the repository.
2.  Create a **new branch** for your feature (`git checkout -b feature/AmazingIdea`).
3.  **Implement** your changes, adhering to the existing code style.
4.  **Test** thoroughly.
5.  **Commit** your changes (`git commit -m 'Add some AmazingIdea'`).
6.  **Push** to the branch (`git push origin feature/AmazingIdea`).
7.  Open a **Pull Request**.

Please ensure your pull requests are well-described and reference any relevant issues.



## Acknowledgments : 

- Deputy Dean of the Georgian Technical University, **Nona Otkhozoria**
- Intel for AVX/SSE2 documentation
- Microsoft for Visual Studio compiler optimizations
- The C++ community for continuous inspiration
- **Georgian Technical University**
- To friends for support

----

## 📄 Citation

If you use Fluminum in your research, please cite:

```bibtex
@software{fluminum,
  title={Fluminum: High-Performance Matrix Operations with Strassen's Algorithm},
  author={Schreiry(David Greve)},
  year={2025},
  version={2.2},
  url={https://github.com/Schreiry/fluminum}
}
```


-----

## 📜 License

This project is licensed under the **MIT License**. See the [LICENSE](https://www.google.com/search?q=LICENSE) file for full details.

[![License: MIT](https://img.shields.io/badge/License-MIT-blue.svg)](https://opensource.org/licenses/MIT)  [![License: CC BY 4.0](https://img.shields.io/badge/License-CC%20BY%204.0-lightgrey.svg)](https://creativecommons.org/licenses/by/4.0/)
-----

The goal of the project, I remind you. training. I am not better than you. You most likely want to hear that I am worse than you. However, I and you cannot judge this. I know what you do not know, and you know what I cannot know.
thank you for your attention, do not judge strictly
