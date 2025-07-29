#include "PerformanceMonitor.h"
#include <iostream>
#include <iomanip>
#include <thread>
#include <pdh.h>
#include <vector>
#include <string>

#pragma comment(lib, "pdh.lib")

// --- Console Formatting from main project (simplified) ---
const std::string PM_RED = "\033[1;31m";
const std::string PM_GREEN = "\033[1;32m";
const std::string PM_YELLOW = "\033[1;33m";
const std::string PM_BLUE = "\033[1;34m";
const std::string PM_PURPLE = "\033[1;35m";
const std::string PM_CYAN = "\033[1;36m";
const std::string PM_RESET = "\033[0m";
const std::string PM_LIGHT_GRAY = "\033[0;37m";


// --- Helper classes for PDH Query Management ---
class PdhQueryManager {
public:
    PdhQueryManager() {
        PdhOpenQuery(nullptr, 0, &queryHandle_);
    }
    ~PdhQueryManager() {
        if (queryHandle_) {
            PdhCloseQuery(queryHandle_);
        }
    }
    PDH_HQUERY getQueryHandle() {
        return queryHandle_;
    }
private:
    PDH_HQUERY queryHandle_ = nullptr;
};

// --- Data Fetching Functions ---

std::string getCpuName() {
    char CPUBrandString[0x40] = { 0 };
    int CPUInfo[4] = { -1 };

    __cpuid(CPUInfo, 0x80000000);
    unsigned int nExIds = CPUInfo[0];

    for (unsigned int i = 0x80000000; i <= nExIds; ++i) {
        __cpuid(CPUInfo, i);
        if (i == 0x80000002)
            memcpy(CPUBrandString, CPUInfo, sizeof(CPUInfo));
        else if (i == 0x80000003)
            memcpy(CPUBrandString + 16, CPUInfo, sizeof(CPUInfo));
        else if (i == 0x80000004)
            memcpy(CPUBrandString + 32, CPUInfo, sizeof(CPUInfo));
    }
    return std::string(CPUBrandString);
}

void getMemoryInfo(unsigned long long& totalRam, unsigned long long& usedRam, double& usagePercentage) {
    MEMORYSTATUSEX statex;
    statex.dwLength = sizeof(statex);
    GlobalMemoryStatusEx(&statex);
    totalRam = statex.ullTotalPhys / (1024 * 1024);
    usedRam = (statex.ullTotalPhys - statex.ullAvailPhys) / (1024 * 1024);
    usagePercentage = statex.dwMemoryLoad;
}

// --- Display Functions ---
void SetConsoleCursorPosition(int x, int y) {
    COORD coord = { (SHORT)x, (SHORT)y };
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);
}

void ClearScreen() {
    HANDLE hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
    COORD coord = { 0, 0 };
    DWORD count;
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    // ÈÑÏÐÀÂËÅÍÍÀß ÑÒÐÎÊÀ: Óáðàíà îïå÷àòêà â èìåíè ôóíêöèè
    GetConsoleScreenBufferInfo(hStdOut, &csbi);
    FillConsoleOutputCharacter(hStdOut, ' ', csbi.dwSize.X * csbi.dwSize.Y, coord, &count);
    SetConsoleCursorPosition(0, 0);
}

void printBar(const std::string& label, double percentage, int width = 50) {
    std::cout << std::setw(15) << std::left << label << " |";
    int pos = static_cast<int>((percentage / 100.0) * width);
    std::string bar;
    if (percentage < 40) bar += PM_GREEN;
    else if (percentage < 75) bar += PM_YELLOW;
    else bar += PM_RED;

    for (int i = 0; i < width; ++i) {
        bar += (i < pos) ? "\xDB" : " ";
    }
    bar += PM_RESET;
    std::cout << bar << "| " << PM_CYAN << std::fixed << std::setprecision(1) << std::setw(5) << std::right << percentage << "%" << PM_RESET << std::endl;
}

void displayPerformance(const PerformanceData& data) {
    SetConsoleCursorPosition(0, 0);

    std::cout << PM_PURPLE << "--- FLUMINUM PERFORMANCE MONITOR ---" << PM_RESET << std::endl;
    std::cout << PM_LIGHT_GRAY << "------------------------------------" << PM_RESET << std::endl;
    std::cout << PM_YELLOW << "Processor:" << PM_RESET << " " << data.cpuName << std::endl;
    std::cout << PM_YELLOW << "Total RAM:" << PM_RESET << " " << data.totalRamMB << " MB" << std::endl;
    std::cout << PM_LIGHT_GRAY << "------------------------------------" << PM_RESET << std::endl << std::endl;

    printBar("CPU Total", data.totalCpuUsage);
    std::cout << std::endl;

    for (int i = 0; i < data.coreCount; ++i) {
        printBar("  Core " + std::to_string(i), data.coreUsage[i]);
    }
    std::cout << std::endl;

    printBar("Memory Usage", data.ramUsagePercentage);

    std::cout << std::endl << PM_LIGHT_GRAY << "Press Ctrl+C to exit this window." << PM_RESET << std::endl;
    std::cout << std::flush;
}


// --- Main Entry for Monitor ---
int RunPerformanceMonitor() {
    SetConsoleTitle(L"Performance Monitor");
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_CURSOR_INFO cursorInfo;
    GetConsoleCursorInfo(hConsole, &cursorInfo);
    cursorInfo.bVisible = false; // Ñêðûòü êóðñîð
    SetConsoleCursorInfo(hConsole, &cursorInfo);


    PdhQueryManager queryManager;
    PDH_HQUERY query = queryManager.getQueryHandle();

    // --- CPU Total Usage Counter ---
    PDH_HCOUNTER totalCpuCounter;
    PdhAddCounter(query, L"\\Processor(_Total)\\% Processor Time", 0, &totalCpuCounter);

    // --- Per Core Usage Counters ---
    SYSTEM_INFO sysInfo;
    GetSystemInfo(&sysInfo);
    int coreCount = sysInfo.dwNumberOfProcessors;
    std::vector<PDH_HCOUNTER> coreCounters(coreCount);
    for (int i = 0; i < coreCount; ++i) {
        std::wstring counterPath = L"\\Processor(" + std::to_wstring(i) + L")\\% Processor Time";
        PdhAddCounter(query, counterPath.c_str(), 0, &coreCounters[i]);
    }

    // Initial collection to establish a baseline
    PdhCollectQueryData(query);
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));

    PerformanceData perfData;
    perfData.cpuName = getCpuName();
    perfData.coreCount = coreCount;
    perfData.coreUsage.resize(coreCount);

    ClearScreen();

    while (true) {
        PdhCollectQueryData(query);

        PDH_FMT_COUNTERVALUE counterValue;

        // Total CPU
        PdhGetFormattedCounterValue(totalCpuCounter, PDH_FMT_DOUBLE, nullptr, &counterValue);
        perfData.totalCpuUsage = counterValue.doubleValue;

        // Per-Core CPU
        for (int i = 0; i < coreCount; ++i) {
            PdhGetFormattedCounterValue(coreCounters[i], PDH_FMT_DOUBLE, nullptr, &counterValue);
            perfData.coreUsage[i] = counterValue.doubleValue;
        }

        // RAM
        getMemoryInfo(perfData.totalRamMB, perfData.usedRamMB, perfData.ramUsagePercentage);

        // Display
        displayPerformance(perfData);

        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }

    return 0;
}