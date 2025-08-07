#include "PerformanceMonitor.h"
#include <iostream>
#include <iomanip>
#include <thread>
#include <stdexcept>
#include <algorithm>
#include <sstream>
#include <chrono>
#include <PdhMsg.h>
#include <algorithm> // дл€ std::clamp

#pragma comment(lib, "pdh.lib")
#undef max
#undef min

// --- ÷ветовые константы ---
enum ConsoleColor { DARK_BLUE = 1, DARK_GREEN = 2, DARK_CYAN = 3, DARK_RED = 4, DARK_MAGENTA = 5, DARK_YELLOW = 6, GRAY = 7, DARK_GRAY = 8, BLUE = 9, GREEN = 10, CYAN = 11, RED = 12, MAGENTA = 13, YELLOW = 14, WHITE = 15 };

PerformanceMonitor::PerformanceMonitor()
    : activeBufferIndex_(0), charBuffer_(nullptr), queryHandle_(nullptr), bufferCoord_({ 0, 0 })
{
    QueryStaticInfo();
    InitConsole();
    InitPdhQueries();
}

PerformanceMonitor::~PerformanceMonitor() {
    if (charBuffer_) delete[] charBuffer_;
    if (queryHandle_) PdhCloseQuery(queryHandle_);
    SetConsoleActiveScreenBuffer(consoleHandles_[1]);
    CONSOLE_CURSOR_INFO cursorInfo;
    GetConsoleCursorInfo(consoleHandles_[0], &cursorInfo);
    cursorInfo.bVisible = TRUE;
    SetConsoleCursorInfo(consoleHandles_[0], &cursorInfo);
}

void PerformanceMonitor::QueryStaticInfo() {
    SYSTEM_INFO sysInfo;
    GetSystemInfo(&sysInfo);
    logicalCoreCount_ = sysInfo.dwNumberOfProcessors;

    MEMORYSTATUSEX statex;
    statex.dwLength = sizeof(statex);
    GlobalMemoryStatusEx(&statex);
    perfData_.totalRamMB = statex.ullTotalPhys / (1024 * 1024);
}

void PerformanceMonitor::InitConsole() {
    SetConsoleTitle(L"Real-Time Performance Monitor");
    SetConsoleOutputCP(CP_UTF8);
    consoleHandles_[1] = GetStdHandle(STD_OUTPUT_HANDLE);
    consoleHandles_[0] = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);
    if (consoleHandles_[0] == INVALID_HANDLE_VALUE) throw std::runtime_error("Failed to create console buffer.");

    short windowHeight = static_cast<short>(8 + logicalCoreCount_);
    if (windowHeight > 40) windowHeight = 40;
    bufferSize_ = { 100, windowHeight };

    SetConsoleScreenBufferSize(consoleHandles_[0], bufferSize_);
    SMALL_RECT windowSize = { 0, 0, bufferSize_.X - 1, bufferSize_.Y - 1 };
    SetConsoleWindowInfo(consoleHandles_[0], TRUE, &windowSize);

    charBuffer_ = new CHAR_INFO[bufferSize_.X * bufferSize_.Y];
    consoleWriteArea_ = { 0, 0, bufferSize_.X - 1, bufferSize_.Y - 1 };
    CONSOLE_CURSOR_INFO cursorInfo;
    GetConsoleCursorInfo(consoleHandles_[0], &cursorInfo);
    cursorInfo.bVisible = FALSE;
    SetConsoleCursorInfo(consoleHandles_[0], &cursorInfo);
    SetConsoleCursorInfo(consoleHandles_[1], &cursorInfo);
}

void PerformanceMonitor::InitPdhQueries() {
    PDH_STATUS status;
    if (PdhOpenQuery(NULL, 0, &queryHandle_) != ERROR_SUCCESS) throw std::runtime_error("PDH Open Query failed.");

    status = PdhAddEnglishCounterW(queryHandle_, L"\\Processor(_Total)\\% Processor Time", 0, &totalCpuCounter_);
    if (status != ERROR_SUCCESS) throw std::runtime_error("PDH Add Total CPU counter failed.");

    coreCounters_.resize(logicalCoreCount_);
    perfData_.coreUsage.resize(logicalCoreCount_);
    for (int i = 0; i < logicalCoreCount_; ++i) {
        std::wstring path = L"\\Processor(" + std::to_wstring(i) + L")\\% Processor Time";
        status = PdhAddEnglishCounterW(queryHandle_, path.c_str(), 0, &coreCounters_[i]);
        if (status != ERROR_SUCCESS) throw std::runtime_error("PDH Add Core " + std::to_string(i) + " counter failed.");
    }

    status = PdhAddEnglishCounterW(queryHandle_, L"\\Memory\\Available MBytes", 0, &availableMemoryCounter_);
    if (status != ERROR_SUCCESS) throw std::runtime_error("PDH Add Available Memory counter failed.");

    status = PdhAddEnglishCounterW(queryHandle_, L"\\Memory\\Page Faults/sec", 0, &pageFaultsCounter_);
    if (status != ERROR_SUCCESS) throw std::runtime_error("PDH Add Page Faults counter failed.");

    PdhCollectQueryData(queryHandle_);
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
}

void PerformanceMonitor::CollectDynamicData() {
    if (PdhCollectQueryData(queryHandle_) != ERROR_SUCCESS) return;
    PDH_FMT_COUNTERVALUE value;
    PdhGetFormattedCounterValue(totalCpuCounter_, PDH_FMT_DOUBLE, NULL, &value);
    perfData_.totalCpuUsage = value.doubleValue;
    for (int i = 0; i < logicalCoreCount_; ++i) {
        PdhGetFormattedCounterValue(coreCounters_[i], PDH_FMT_DOUBLE, NULL, &value);
        perfData_.coreUsage[i] = value.doubleValue;
    }
    PdhGetFormattedCounterValue(availableMemoryCounter_, PDH_FMT_LARGE, NULL, &value);
    perfData_.availableRamMB = value.largeValue;
    PdhGetFormattedCounterValue(pageFaultsCounter_, PDH_FMT_DOUBLE, NULL, &value);
    perfData_.pageFaultsPerSec = value.doubleValue;
}

void PerformanceMonitor::PrintToBuffer(int x, int y, const std::string& text) { if (y >= bufferSize_.Y || x < 0) return; WORD attribute = GRAY; bool in_escape = false; std::string code; int current_x = x; for (char c : text) { if (current_x >= bufferSize_.X) break; if (c == '\033') { in_escape = true; code.clear(); } else if (in_escape) { if (c == 'm') { in_escape = false; if (code == "[1;31") attribute = RED; else if (code == "[1;32") attribute = GREEN; else if (code == "[1;33") attribute = YELLOW; else if (code == "[1;34") attribute = BLUE; else if (code == "[1;35") attribute = MAGENTA; else if (code == "[1;36") attribute = CYAN; else if (code == "[0;37") attribute = GRAY; else if (code == "[1;90") attribute = DARK_GRAY; else if (code == "[0") attribute = GRAY; else if (code == "[1;97") attribute = WHITE; } else { code += c; } } else { if (current_x >= 0) { if (static_cast<unsigned char>(c) >= 0x80) charBuffer_[y * bufferSize_.X + current_x].Char.UnicodeChar = c; else charBuffer_[y * bufferSize_.X + current_x].Char.AsciiChar = c; charBuffer_[y * bufferSize_.X + current_x].Attributes = attribute; } current_x++; } } }
void PerformanceMonitor::PrintBar(int x, int y, double percentage, const std::string& label) { const int width = 10; std::stringstream ss; double clamped_val = std::clamp(percentage, 0.0, 100.0); std::stringstream label_ss; label_ss << label << " (" << std::fixed << std::setprecision(1) << std::setw(4) << clamped_val << "%)"; ss << "\033[1;97m" << std::left << std::setw(22) << label_ss.str() << "\033[0;37m["; int bar_fill = static_cast<int>(clamped_val / 10.0 + 0.5); if (clamped_val >= 75.0) ss << "\033[1;31m"; else if (clamped_val >= 40.0) ss << "\033[1;33m"; else ss << "\033[1;32m"; for (int i = 0; i < bar_fill; ++i) ss << "\u2588"; ss << "\033[1;90m"; for (int i = bar_fill; i < width; ++i) ss << "\u2588"; ss << "\033[0;37m]"; PrintToBuffer(x, y, ss.str()); }

void PerformanceMonitor::Render() {
    for (int i = 0; i < bufferSize_.X * bufferSize_.Y; ++i) {
        charBuffer_[i].Char.AsciiChar = ' ';
        charBuffer_[i].Attributes = 0;
    }

    std::string title = " REAL-TIME MONITOR ";
    PrintToBuffer((bufferSize_.X - title.length()) / 2, 1, "\033[1;35m" + title);

    int y = 3;
    int col1_x = 4;
    int col2_x = bufferSize_.X / 2 + 5;

    PrintToBuffer(col1_x, y, "\033[1;32mCPU USAGE");
    PrintToBuffer(col2_x, y, "\033[1;34mMEMORY");
    y += 2;

    int max_rows = logicalCoreCount_ + 1;
    for (int i = 0; i < max_rows; ++i) {
        if (y + i >= bufferSize_.Y - 1) break;
        int current_y = y + i;

        if (i == 0) PrintBar(col1_x, current_y, perfData_.totalCpuUsage, "CPU Total");
        else if ((i - 1) < logicalCoreCount_) PrintBar(col1_x + 2, current_y, perfData_.coreUsage[i - 1], "Core " + std::to_string(i - 1));

        if (i == 0) {
            double ramUsagePercent = (perfData_.totalRamMB > 0) ? (((double)perfData_.totalRamMB - perfData_.availableRamMB) / perfData_.totalRamMB) * 100.0 : 0.0;
            PrintBar(col2_x, current_y, ramUsagePercent, "RAM Usage");
        }
        else if (i == 1) {
            std::stringstream ram_details;
            ram_details << "\033[0;37m" << (perfData_.totalRamMB - perfData_.availableRamMB) << " MB / " << perfData_.totalRamMB << " MB";
            PrintToBuffer(col2_x, current_y, ram_details.str());
        }
        else if (i == 3) {
            PrintToBuffer(col2_x, current_y, "\033[1;97mPage Faults\033[0;37m: \033[1;36m" + std::to_string(static_cast<int>(perfData_.pageFaultsPerSec)) + "/s");
        }
    }

    WriteConsoleOutputA(consoleHandles_[activeBufferIndex_], charBuffer_, bufferSize_, bufferCoord_, &consoleWriteArea_);
    SetConsoleActiveScreenBuffer(consoleHandles_[activeBufferIndex_]);
    activeBufferIndex_ = 1 - activeBufferIndex_;
}

void PerformanceMonitor::Run() {
    while (true) {
        HWND console_wnd = GetConsoleWindow();
        if (console_wnd == NULL || !IsWindowVisible(console_wnd)) { break; }
        CollectDynamicData();
        Render();
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }
}

int RunPerformanceMonitorEntry() {
    try {
        PerformanceMonitor monitor;
        monitor.Run();
    }
    catch (const std::exception& e) {
        std::cerr << "A critical error occurred in Monitor: " << e.what() << std::endl;
        std::cin.get();
        return 1;
    }
    return 0;
}