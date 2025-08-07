#pragma once

// --- Standard Library Includes ---
#include <windows.h>
#include <iostream>
#include <vector>
#include <string>
#include <thread>
#include <stdexcept>
#include <random>
#include <iomanip>
#include <cmath>
#include <future>
#include <chrono>
#include <atomic>
#include <algorithm>
#include <limits>
#include <functional>
#include <fstream>
#include <sstream>
#include <cctype>
#include <type_traits>
#include <queue>
#include <mutex>
#include <condition_variable>

// --- SIMD Intrinsics ---
#ifdef _MSC_VER
#include <intrin.h>
#endif
#if defined(__AVX__) || (defined(_MSC_VER) && defined(__AVX__))
#include <immintrin.h>
#define HAS_AVX
const int SIMD_VECTOR_SIZE_DOUBLE = 4;
#elif defined(__SSE2__) || (defined(_MSC_VER) && defined(__SSE2__))
#include <emmintrin.h>
#define HAS_SSE2
const int SIMD_VECTOR_SIZE_DOUBLE = 2;
#else
const int SIMD_VECTOR_SIZE_DOUBLE = 1;
#endif

#pragma comment(lib, "Psapi.lib")

// --- Global Using Declarations ---
using std::string;
using std::cout;
using std::cin;
using std::endl;
using std::cerr;

// --- Console Formatting Constants ---
const string RED = "\033[1;31m";
const string GREEN = "\033[1;32m";
const string YELLOW = "\033[1;33m";
const string BLUE = "\033[1;34m";
const string PURPLE = "\033[1;35m";
const string CYAN = "\033[1;36m";
const string RESET = "\033[0m";
const string DARK_GRAY = "\033[0;90m";
const string LIGHT_GRAY = "\033[0;37m";

const string BOX_HLINE = "\u2500";
const string BOX_VLINE = "\u2502";
const string BOX_TLCORNER = "\u250C";
const string BOX_TRCORNER = "\u2510";
const string BOX_BLCORNER = "\u2514";
const string BOX_BRCORNER = "\u2518";
const string BOX_LTEE = "\u251C";
const string BOX_RTEE = "\u2524";
const string BOX_BTEE = "\u2534";
const string BOX_TTEE = "\u252C";
const string BOX_CROSS = "\u253C";

// --- Enums and Structs ---
enum class Alignment { Left, Center, Right };

// Forward declaration of Matrix class to resolve dependencies
class Matrix;

// Structs for system info and results
struct SystemMemoryInfo {
    unsigned long long totalPhysicalMB;
    unsigned long long availablePhysicalMB;
};

struct ProcessMemoryInfo {
    size_t peakWorkingSetMB;
};

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

    MultiplicationResult(); // Constructor defined in Algorithms.cpp
};


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

    ComparisonResult(); // Constructor defined in Algorithms.cpp
};