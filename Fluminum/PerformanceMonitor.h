#pragma once

#include <windows.h>
#include <string>
#include <vector>
#include <pdh.h>

// --- Структуры для динамических данных ---

struct PerformanceData {
    double totalCpuUsage = 0.0;
    std::vector<double> coreUsage;
    unsigned long long totalRamMB = 0;
    unsigned long long availableRamMB = 0;
    double pageFaultsPerSec = 0.0;
};

// --- Основной класс монитора производительности ---

class PerformanceMonitor {
public:
    PerformanceMonitor();
    ~PerformanceMonitor();

    PerformanceMonitor(const PerformanceMonitor&) = delete;
    PerformanceMonitor& operator=(const PerformanceMonitor&) = delete;

    void Run();

private:
    // --- Методы инициализации и сбора данных ---
    void InitConsole();
    void QueryStaticInfo();
    void InitPdhQueries();
    void CollectDynamicData();

    // --- Методы рендеринга ---
    void Render();
    void PrintToBuffer(int x, int y, const std::string& text);
    void PrintBar(int x, int y, double percentage, const std::string& label);

    // --- Дескрипторы и буферы консоли ---
    HANDLE consoleHandles_[2];
    int activeBufferIndex_;
    CHAR_INFO* charBuffer_;
    COORD bufferSize_;
    COORD bufferCoord_;
    SMALL_RECT consoleWriteArea_;

    // --- Хранилища данных ---
    PerformanceData perfData_;
    int logicalCoreCount_ = 0;

    // --- Дескрипторы PDH для динамических данных ---
    PDH_HQUERY queryHandle_;
    PDH_HCOUNTER totalCpuCounter_;
    std::vector<PDH_HCOUNTER> coreCounters_;
    PDH_HCOUNTER availableMemoryCounter_;
    PDH_HCOUNTER pageFaultsCounter_;
};

int RunPerformanceMonitorEntry();