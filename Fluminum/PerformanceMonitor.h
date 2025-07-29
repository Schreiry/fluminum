#pragma once

// --- Includes for Performance Monitor ---
#include <windows.h>
#include <string>
#include <vector>

// Structure to hold all performance data
struct PerformanceData {
    std::string cpuName;
    int coreCount;
    double totalCpuUsage;
    std::vector<double> coreUsage;
    unsigned long long totalRamMB;
    unsigned long long usedRamMB;
    double ramUsagePercentage;
};

// --- Function Declarations for the Performance Monitor ---

/**
 * @brief Главная функция для окна мониторинга производительности.
 *
 * Запускает цикл, который непрерывно собирает и отображает системные метрики.
 * Эта функция предназначена для запуска в отдельном процессе.
 * @return Код выхода (0 при успехе).
 */
int RunPerformanceMonitor();