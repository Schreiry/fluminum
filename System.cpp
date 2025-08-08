#include "System.h"
#include "PerformanceMonitor.h" // For RunPerformanceMonitorEntry
#include "Matrix.h" // Needed for auto-tuning

// --- Global Variables ---
LARGE_INTEGER g_performanceFrequency = { 0 };
bool has_avx_global = false;
bool has_sse2_global = false;
int G_OPTIMAL_TILE_SIZE = 32; // Default, will be overwritten by auto-tuner

// --- System Information ---
SystemMemoryInfo getSystemMemoryInfo() {
    MEMORYSTATUSEX statex;
    statex.dwLength = sizeof(statex);
    if (!GlobalMemoryStatusEx(&statex)) {
        DWORD error = GetLastError();
        cerr << RED << "Warning: Failed to get system memory status (Error " << error << "). Reporting 0 MB." << RESET << endl;
        return { 0, 0 };
    }
    return { statex.ullTotalPhys / (1024 * 1024), statex.ullAvailPhys / (1024 * 1024) };
}

unsigned int getCpuCoreCount() {
    unsigned int cores = std::thread::hardware_concurrency();
    return (cores == 0) ? 1 : cores;
}

ProcessMemoryInfo getProcessMemoryUsage() {
    PROCESS_MEMORY_COUNTERS pmc;
    ZeroMemory(&pmc, sizeof(pmc));
    pmc.cb = sizeof(pmc);
    if (GetProcessMemoryInfo(GetCurrentProcess(), &pmc, sizeof(pmc))) {
        return { pmc.PeakWorkingSetSize / (1024 * 1024) };
    }
    else {
        DWORD error = GetLastError();
        cerr << RED << "Warning: Failed to get process memory info (Error " << error << "). Reporting 0 MB." << RESET << endl;
        return { 0 };
    }
}

// --- Performance Counter ---
void initializePerformanceCounter() {
    if (!QueryPerformanceFrequency(&g_performanceFrequency)) {
        cerr << RED << "Warning: QueryPerformanceFrequency failed. High-resolution timing may be unavailable." << RESET << endl;
        g_performanceFrequency.QuadPart = 0;
    }
}

// --- SIMD Support ---
void check_simd_support() {
#ifdef _MSC_VER
    int cpuInfo[4];
    __cpuidex(cpuInfo, 1, 0);
    has_sse2_global = (cpuInfo[3] & (1 << 26)) != 0;

    bool osxsave_supported = (cpuInfo[2] & (1 << 27)) != 0;
    bool avx_cpu_supported = (cpuInfo[2] & (1 << 28)) != 0;

    has_avx_global = false;
    if (avx_cpu_supported && osxsave_supported) {
        unsigned __int64 xcrFeatureMask = _xgetbv(0);
        if ((xcrFeatureMask & 6) == 6) {
            has_avx_global = true;
        }
    }
#elif defined(__GNUC__) || defined(__clang__)
#ifdef __AVX__
    has_avx_global = true;
#endif
#ifdef __SSE2__
    has_sse2_global = true;
#endif
#else
    has_avx_global = false;
    has_sse2_global = false;
#endif
}


// --- NEW: Tiling Auto-Tuner Implementation ---
void autoTuneTileSize() {
    cout << CYAN << "Performing one-time hardware tuning for optimal tile size..." << RESET << endl;

    const int test_dim = 256; // Small enough to be fast, large enough to be meaningful
    const int num_runs = 3; // Number of runs to average for each tile size
    Matrix A = Matrix::generateRandom(test_dim, test_dim);
    Matrix B = Matrix::generateRandom(test_dim, test_dim);

    std::vector<int> tile_sizes_to_test = { 16, 24, 32, 48, 64, 96, 128 };
    double best_time = std::numeric_limits<double>::max();
    int best_tile_size = 32;

    cout << "Benchmarking tile sizes: ";
    for (int size : tile_sizes_to_test) {
        cout << size << "... " << std::flush;
        if (size > test_dim) continue;

        double total_duration = 0;
        for (int i = 0; i < num_runs; ++i) {
            auto start = std::chrono::high_resolution_clock::now();
            A.multiply_tiled(B, size); // Use the actual tiled method
            auto end = std::chrono::high_resolution_clock::now();
            total_duration += std::chrono::duration<double>(end - start).count();
        }
        double avg_time = total_duration / num_runs;

        if (avg_time < best_time) {
            best_time = avg_time;
            best_tile_size = size;
        }
    }

    G_OPTIMAL_TILE_SIZE = best_tile_size;
    cout << endl << GREEN << "Auto-tuning complete. Optimal tile size for this system: " << G_OPTIMAL_TILE_SIZE << "x" << G_OPTIMAL_TILE_SIZE << RESET << endl << endl;
}


// --- Memory Estimation ---
unsigned long long estimateStrassenMemoryMB(int n_padded) {
    if (n_padded <= 0) return 0;
    unsigned long long elementSize = sizeof(double);
    unsigned long long numElementsPerPaddedMatrix = static_cast<unsigned long long>(n_padded) * static_cast<unsigned long long>(n_padded);
    // Strassen requires ~18 matrices of the padded size in memory in the worst-case recursive stack
    unsigned long long estimatedTotalElements = numElementsPerPaddedMatrix * 18;
    unsigned long long estimatedTotalBytes = estimatedTotalElements * elementSize;
    return estimatedTotalBytes / (1024 * 1024);
}

unsigned long long estimateComparisonMemoryMB(int n_padded) {
    if (n_padded <= 0) return 0;
    unsigned long long elementSize = sizeof(double);
    unsigned long long numElementsPerPaddedMatrix = static_cast<unsigned long long>(n_padded) * static_cast<unsigned long long>(n_padded);
    // Comparison requires 2 original + 1 result = 3 matrices
    unsigned long long estimatedTotalElements = numElementsPerPaddedMatrix * 3;
    unsigned long long estimatedTotalBytes = estimatedTotalElements * elementSize;
    return estimatedTotalBytes / (1024 * 1024);
}


// --- Process Management ---
void LaunchMonitorProcess() {
    STARTUPINFOA si;
    PROCESS_INFORMATION pi;

    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));

    char selfPath[MAX_PATH];
    if (!GetModuleFileNameA(NULL, selfPath, MAX_PATH)) {
        cerr << RED << "Error: GetModuleFileNameA failed. Cannot launch monitor." << RESET << endl;
        return;
    }

    string commandLine = "\"";
    commandLine += selfPath;
    commandLine += "\" --monitor";

    std::vector<char> commandLineVec(commandLine.begin(), commandLine.end());
    commandLineVec.push_back('\0');

    if (!CreateProcessA(
        NULL,
        commandLineVec.data(),
        NULL,
        NULL,
        FALSE,
        CREATE_NEW_CONSOLE,
        NULL,
        NULL,
        &si,
        &pi)
        )
    {
        cerr << RED << "Error: CreateProcess failed (" << GetLastError() << "). Cannot launch monitor." << RESET << endl;
        return;
    }

    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
}