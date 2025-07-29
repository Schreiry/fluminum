#pragma once

#include <windows.h>
#include <string>
#include <vector>
#include <pdh.h>

// --- Структуры для хранения данных ---

// Статическая информация о кэше процессора
struct CacheInfo {
    DWORD level;
    DWORD size; // в байтах
    DWORD lineSize;
    DWORD associativity;
};

// Статическая информация о системе (собирается один раз)
struct StaticSystemInfo {
    std::string cpuName;
    int logicalCoreCount;
    std::vector<CacheInfo> caches;
};

// Динамические данные о производительности (обновляются постоянно)
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

    // Запрещаем копирование, чтобы избежать проблем с дескрипторами
    PerformanceMonitor(const PerformanceMonitor&) = delete;
    PerformanceMonitor& operator=(const PerformanceMonitor&) = delete;

    // Главный цикл монитора
    void Run();

private:
    // --- Методы инициализации ---
    void InitConsole();
    void InitPdhQueries();
    void QueryStaticInfo();

    // --- Методы сбора данных ---
    void CollectDynamicData();
    bool RestartPdhQuery();

    // --- Методы рендеринга ---
    void Render();
    void PrintToBuffer(int x, int y, const std::string& text);
    void PrintBar(int x, int y, int width, double percentage, const std::string& label);

    // --- Консольные буферы для плавной отрисовки ---
    HANDLE consoleHandles_[2]; // 0 - back buffer, 1 - front buffer
    int activeBufferIndex_;
    CHAR_INFO* charBuffer_;
    COORD bufferSize_;
    COORD bufferCoord_;
    SMALL_RECT consoleWriteArea_;

    // --- Системная информация ---
    StaticSystemInfo staticInfo_;
    PerformanceData perfData_;

    // --- Объекты для PDH ---
    PDH_HQUERY queryHandle_;
    PDH_HCOUNTER totalCpuCounter_;
    std::vector<PDH_HCOUNTER> coreCounters_;
    PDH_HCOUNTER availableMemoryCounter_;
    PDH_HCOUNTER pageFaultsCounter_;
};

// --- Точка входа для процесса мониторинга ---
int RunPerformanceMonitorEntry();