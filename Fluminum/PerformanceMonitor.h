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
 * @brief ������� ������� ��� ���� ����������� ������������������.
 *
 * ��������� ����, ������� ���������� �������� � ���������� ��������� �������.
 * ��� ������� ������������� ��� ������� � ��������� ��������.
 * @return ��� ������ (0 ��� ������).
 */
int RunPerformanceMonitor();