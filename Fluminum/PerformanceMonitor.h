#pragma once

#include <windows.h>
#include <string>
#include <vector>
#include <pdh.h>

// --- ��������� ��� �������� ������ ---

// ����������� ���������� � ���� ����������
struct CacheInfo {
    DWORD level;
    DWORD size; // � ������
    DWORD lineSize;
    DWORD associativity;
};

// ����������� ���������� � ������� (���������� ���� ���)
struct StaticSystemInfo {
    std::string cpuName;
    int logicalCoreCount;
    std::vector<CacheInfo> caches;
};

// ������������ ������ � ������������������ (����������� ���������)
struct PerformanceData {
    double totalCpuUsage = 0.0;
    std::vector<double> coreUsage;
    unsigned long long totalRamMB = 0;
    unsigned long long availableRamMB = 0;
    double pageFaultsPerSec = 0.0;
};

// --- �������� ����� �������� ������������������ ---

class PerformanceMonitor {
public:
    PerformanceMonitor();
    ~PerformanceMonitor();

    // ��������� �����������, ����� �������� ������� � �������������
    PerformanceMonitor(const PerformanceMonitor&) = delete;
    PerformanceMonitor& operator=(const PerformanceMonitor&) = delete;

    // ������� ���� ��������
    void Run();

private:
    // --- ������ ������������� ---
    void InitConsole();
    void InitPdhQueries();
    void QueryStaticInfo();

    // --- ������ ����� ������ ---
    void CollectDynamicData();
    bool RestartPdhQuery();

    // --- ������ ���������� ---
    void Render();
    void PrintToBuffer(int x, int y, const std::string& text);
    void PrintBar(int x, int y, int width, double percentage, const std::string& label);

    // --- ���������� ������ ��� ������� ��������� ---
    HANDLE consoleHandles_[2]; // 0 - back buffer, 1 - front buffer
    int activeBufferIndex_;
    CHAR_INFO* charBuffer_;
    COORD bufferSize_;
    COORD bufferCoord_;
    SMALL_RECT consoleWriteArea_;

    // --- ��������� ���������� ---
    StaticSystemInfo staticInfo_;
    PerformanceData perfData_;

    // --- ������� ��� PDH ---
    PDH_HQUERY queryHandle_;
    PDH_HCOUNTER totalCpuCounter_;
    std::vector<PDH_HCOUNTER> coreCounters_;
    PDH_HCOUNTER availableMemoryCounter_;
    PDH_HCOUNTER pageFaultsCounter_;
};

// --- ����� ����� ��� �������� ����������� ---
int RunPerformanceMonitorEntry();