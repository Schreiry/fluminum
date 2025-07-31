#include "PerformanceMonitor.h"
#include <iostream>
#include <iomanip>
#include <thread>
#include <stdexcept>
#include <algorithm>
#include <sstream>
#include <chrono>
#include <PdhMsg.h>
#include <map>
#include <algorithm> // для std::clamp

#pragma comment(lib, "pdh.lib")
#undef max
#undef min

// --- Цветовые константы для консоли ---
enum ConsoleColor {
    DARK_BLUE = 1, BLUE = 9,
    DARK_GREEN = 2, GREEN = 10,
    DARK_CYAN = 3, CYAN = 11,
    DARK_RED = 4, RED = 12,
    DARK_MAGENTA = 5, MAGENTA = 13,
    DARK_YELLOW = 6, YELLOW = 14,
    GRAY = 7,
    DARK_GRAY = 8,
    WHITE = 15
};

// --- Реализация класса PerformanceMonitor ---

PerformanceMonitor::PerformanceMonitor()
    : activeBufferIndex_(0), charBuffer_(nullptr), queryHandle_(nullptr), bufferCoord_({ 0, 0 })
{
    QueryStaticInfo(); // Сначала получаем информацию, чтобы правильно задать высоту окна
    InitConsole();
    InitPdhQueries();
    perfData_.coreUsage.resize(staticInfo_.logicalCoreCount);
}

PerformanceMonitor::~PerformanceMonitor() {
    if (charBuffer_) {
        delete[] charBuffer_;
    }
    if (queryHandle_) {
        PdhCloseQuery(queryHandle_);
    }
    SetConsoleActiveScreenBuffer(consoleHandles_[1]);
    CONSOLE_CURSOR_INFO cursorInfo;
    GetConsoleCursorInfo(consoleHandles_[0], &cursorInfo);
    cursorInfo.bVisible = TRUE;
    SetConsoleCursorInfo(consoleHandles_[0], &cursorInfo);
}

void PerformanceMonitor::InitConsole() {
    SetConsoleTitle(L"Performance Monitor");
    SetConsoleOutputCP(CP_UTF8);

    consoleHandles_[1] = GetStdHandle(STD_OUTPUT_HANDLE);
    consoleHandles_[0] = CreateConsoleScreenBuffer(
        GENERIC_READ | GENERIC_WRITE,
        FILE_SHARE_READ | FILE_SHARE_WRITE,
        NULL, CONSOLE_TEXTMODE_BUFFER, NULL
    );

    if (consoleHandles_[0] == INVALID_HANDLE_VALUE || consoleHandles_[1] == INVALID_HANDLE_VALUE) {
        throw std::runtime_error("Failed to create console screen buffers.");
    }

    // Динамически вычисляем высоту окна в зависимости от количества ядер
    short windowHeight = static_cast<short>(15 + staticInfo_.logicalCoreCount);
    if (windowHeight > 45) windowHeight = 45; // Ограничим максимальную высоту
    bufferSize_ = { 120, windowHeight };

    SetConsoleScreenBufferSize(consoleHandles_[0], bufferSize_);

    SMALL_RECT windowSize = { 0, 0, bufferSize_.X - 1, bufferSize_.Y - 1 };
    SetConsoleWindowInfo(consoleHandles_[0], TRUE, &windowSize);
    SetConsoleWindowInfo(consoleHandles_[1], TRUE, &windowSize);

    charBuffer_ = new CHAR_INFO[bufferSize_.X * bufferSize_.Y];
    consoleWriteArea_ = { 0, 0, bufferSize_.X - 1, bufferSize_.Y - 1 };

    CONSOLE_CURSOR_INFO cursorInfo;
    GetConsoleCursorInfo(consoleHandles_[0], &cursorInfo);
    cursorInfo.bVisible = FALSE;
    SetConsoleCursorInfo(consoleHandles_[0], &cursorInfo);
    SetConsoleCursorInfo(consoleHandles_[1], &cursorInfo);
}

void PerformanceMonitor::QueryStaticInfo() {
    char cpuNameStr[0x40] = { 0 };
    int cpuInfo[4] = { -1 };
    __cpuid(cpuInfo, 0x80000000);
    unsigned int nExIds = cpuInfo[0];
    for (unsigned int i = 0x80000000; i <= nExIds; ++i) {
        __cpuid(cpuInfo, i);
        if (i == 0x80000002) memcpy(cpuNameStr, cpuInfo, sizeof(cpuInfo));
        else if (i == 0x80000003) memcpy(cpuNameStr + 16, cpuInfo, sizeof(cpuInfo));
        else if (i == 0x80000004) memcpy(cpuNameStr + 32, cpuInfo, sizeof(cpuInfo));
    }
    staticInfo_.cpuName = std::string(cpuNameStr);

    SYSTEM_INFO sysInfo;
    GetSystemInfo(&sysInfo);
    staticInfo_.logicalCoreCount = sysInfo.dwNumberOfProcessors;

    DWORD bufferSize = 0;
    GetLogicalProcessorInformationEx(RelationCache, nullptr, &bufferSize);
    if (bufferSize > 0) {
        std::vector<char> buffer(bufferSize);
        auto* procInfo = reinterpret_cast<PSYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX>(buffer.data());
        if (GetLogicalProcessorInformationEx(RelationCache, procInfo, &bufferSize)) {
            char* ptr = buffer.data();
            while (ptr < buffer.data() + bufferSize) {
                auto* current = reinterpret_cast<PSYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX>(ptr);
                if (current->Relationship == RelationCache && current->Cache.Level != 0) {
                    staticInfo_.caches.push_back({ current->Cache.Level, current->Cache.CacheSize, current->Cache.LineSize, current->Cache.Associativity });
                }
                ptr += current->Size;
            }
        }
    }

    MEMORYSTATUSEX statex;
    statex.dwLength = sizeof(statex);
    GlobalMemoryStatusEx(&statex);
    perfData_.totalRamMB = statex.ullTotalPhys / (1024 * 1024);
}

void PerformanceMonitor::InitPdhQueries() {
    PDH_STATUS status;
    if (PdhOpenQuery(NULL, 0, &queryHandle_) != ERROR_SUCCESS) {
        throw std::runtime_error("Failed to open PDH Query.");
    }
    status = PdhAddEnglishCounterW(queryHandle_, L"\\Processor(_Total)\\% Processor Time", 0, &totalCpuCounter_);
    if (status != ERROR_SUCCESS) throw std::runtime_error("Failed to add Total CPU counter.");
    coreCounters_.resize(staticInfo_.logicalCoreCount);
    for (int i = 0; i < staticInfo_.logicalCoreCount; ++i) {
        std::wstring path = L"\\Processor(" + std::to_wstring(i) + L")\\% Processor Time";
        status = PdhAddEnglishCounterW(queryHandle_, path.c_str(), 0, &coreCounters_[i]);
        if (status != ERROR_SUCCESS) throw std::runtime_error("Failed to add CPU Core " + std::to_string(i) + " counter.");
    }
    status = PdhAddEnglishCounterW(queryHandle_, L"\\Memory\\Available MBytes", 0, &availableMemoryCounter_);
    if (status != ERROR_SUCCESS) throw std::runtime_error("Failed to add Available Memory counter.");
    status = PdhAddEnglishCounterW(queryHandle_, L"\\Memory\\Page Faults/sec", 0, &pageFaultsCounter_);
    if (status != ERROR_SUCCESS) throw std::runtime_error("Failed to add Page Faults counter.");

    PdhCollectQueryData(queryHandle_);
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
}

bool PerformanceMonitor::RestartPdhQuery() {
    if (queryHandle_) PdhCloseQuery(queryHandle_);
    queryHandle_ = nullptr;
    try {
        InitPdhQueries();
    }
    catch (...) {
        return false;
    }
    return true;
}

void PerformanceMonitor::CollectDynamicData() {
    PDH_STATUS status = PdhCollectQueryData(queryHandle_);
    if (status != ERROR_SUCCESS) {
        std::stringstream err;
        err << "PDH collect error: 0x" << std::hex << status;
        PrintToBuffer(2, bufferSize_.Y - 2, "\033[1;31m" + err.str());
        if (!RestartPdhQuery()) {
            PrintToBuffer(2, bufferSize_.Y - 1, "\033[1;31mFATAL: Query restart failed.");
        }
        return;
    }

    PDH_FMT_COUNTERVALUE value;
    status = PdhGetFormattedCounterValue(totalCpuCounter_, PDH_FMT_DOUBLE, NULL, &value);
    perfData_.totalCpuUsage = (status == ERROR_SUCCESS) ? value.doubleValue : 0.0;
    for (int i = 0; i < staticInfo_.logicalCoreCount; ++i) {
        status = PdhGetFormattedCounterValue(coreCounters_[i], PDH_FMT_DOUBLE, NULL, &value);
        perfData_.coreUsage[i] = (status == ERROR_SUCCESS) ? value.doubleValue : 0.0;
    }
    status = PdhGetFormattedCounterValue(availableMemoryCounter_, PDH_FMT_LARGE, NULL, &value);
    perfData_.availableRamMB = (status == ERROR_SUCCESS) ? value.largeValue : 0;
    status = PdhGetFormattedCounterValue(pageFaultsCounter_, PDH_FMT_DOUBLE, NULL, &value);
    perfData_.pageFaultsPerSec = (status == ERROR_SUCCESS) ? value.doubleValue : 0.0;
}

void PerformanceMonitor::PrintToBuffer(int x, int y, const std::string& text) {
    if (y >= bufferSize_.Y || x < 0) return;
    WORD attribute = GRAY;
    bool in_escape = false;
    std::string code;
    int current_x = x;
    for (char c : text) {
        if (current_x >= bufferSize_.X) break;
        if (c == '\033') {
            in_escape = true;
            code.clear();
        }
        else if (in_escape) {
            if (c == 'm') {
                in_escape = false;
                if (code == "[1;31") attribute = RED;
                else if (code == "[1;32") attribute = GREEN;
                else if (code == "[1;33") attribute = YELLOW;
                else if (code == "[1;34") attribute = BLUE;
                else if (code == "[1;35") attribute = MAGENTA;
                else if (code == "[1;36") attribute = CYAN;
                else if (code == "[0;37") attribute = GRAY;
                else if (code == "[1;90") attribute = DARK_GRAY;
                else if (code == "[0") attribute = GRAY;
                else if (code == "[1;97") attribute = WHITE;
            }
            else { code += c; }
        }
        else {
            if (current_x >= 0) {
                if (static_cast<unsigned char>(c) >= 0x80) {
                    charBuffer_[y * bufferSize_.X + current_x].Char.UnicodeChar = c;
                }
                else {
                    charBuffer_[y * bufferSize_.X + current_x].Char.AsciiChar = c;
                }
                charBuffer_[y * bufferSize_.X + current_x].Attributes = attribute;
            }
            current_x++;
        }
    }
}

// ИСПРАВЛЕНО: Функция теперь всегда выводит проценты и использует фиксированную ширину бара
void PerformanceMonitor::PrintBar(int x, int y, double percentage, const std::string& label) {
    const int width = 10; // Ширина бара в 10 символов по требованию
    std::stringstream ss;

    double clamped_val = std::clamp(percentage, 0.0, 100.0);

    // Формируем лейбл с процентами в скобках
    std::stringstream label_ss;
    label_ss << label << " (" << std::fixed << std::setprecision(1) << std::setw(4) << clamped_val << "%)";

    // Выводим лейбл и открывающую скобку бара
    ss << "\033[1;97m" << std::left << std::setw(22) << label_ss.str() << "\033[0;37m[";

    // Каждый символ - 10%. Округляем до ближайшего целого.
    int bar_fill = static_cast<int>(clamped_val / 10.0 + 0.5);

    // Определяем цвет на основе загрузки
    if (clamped_val >= 75.0) ss << "\033[1;31m";      // Red
    else if (clamped_val >= 40.0) ss << "\033[1;33m"; // Yellow
    else ss << "\033[1;32m";                         // Green

    // Рисуем заполненную часть
    for (int i = 0; i < bar_fill; ++i) ss << "\u2588";

    // Рисуем незаполненную часть
    ss << "\033[1;90m"; // Dark Gray
    for (int i = bar_fill; i < width; ++i) ss << "\u2588";

    // Завершаем строку
    ss << "\033[0;37m]";

    PrintToBuffer(x, y, ss.str());
}

// ИСПРАВЛЕНО: Финальная версия рендера с корректной вёрсткой
void PerformanceMonitor::Render() {
    // 1. Очистка буфера
    for (int i = 0; i < bufferSize_.X * bufferSize_.Y; ++i) {
        charBuffer_[i].Char.AsciiChar = ' ';
        charBuffer_[i].Attributes = 0; // Черный фон
    }

    // 2. Заголовок
    std::string title = " FLUMINUM PERFORMANCE MONITOR ";
    PrintToBuffer((bufferSize_.X - title.length()) / 2, 1, "\033[1;35m" + title);

    // 3. Секция System Info
    std::string hline(bufferSize_.X - 4, '\u2500');
    std::string separator_line = "\033[1;90m" + hline;

    PrintToBuffer(4, 2, "\033[1;33mSYSTEM INFO");
    PrintToBuffer(2, 3, separator_line);

    PrintToBuffer(4, 4, "\033[0;37mProcessor: \033[1;97m" + staticInfo_.cpuName);
    std::map<DWORD, DWORD> cache_sizes;
    for (const auto& cache : staticInfo_.caches) { cache_sizes[cache.level] += cache.size; }
    std::stringstream cache_ss;
    cache_ss << "\033[0;37mCaches: ";
    for (auto const& [level, size] : cache_sizes) {
        cache_ss << "\033[1;97m" << "L" << level << ": " << size / 1024 << "K   ";
    }
    PrintToBuffer(4, 5, cache_ss.str());

    // 4. Основные панели
    int y = 7;
    PrintToBuffer(2, y++, separator_line);

    int left_col_x = 4;
    int right_col_x = bufferSize_.X / 2 + 10;

    PrintToBuffer(left_col_x, y, "\033[1;32mCPU USAGE");
    PrintToBuffer(right_col_x, y, "\033[1;34mMEMORY");
    y += 2;

    // --- Левая колонка: CPU ---
    int current_y = y;
    PrintBar(left_col_x, current_y++, perfData_.totalCpuUsage, "CPU Total");
    current_y++; // Пробел
    for (int i = 0; i < staticInfo_.logicalCoreCount; ++i) {
        if (current_y >= bufferSize_.Y - 1) break;
        std::string core_label = "Core " + std::to_string(i);
        PrintBar(left_col_x + 2, current_y++, perfData_.coreUsage[i], core_label);
    }

    // --- Правая колонка: Memory ---
    current_y = y;
    double usedRamMB = perfData_.totalRamMB - perfData_.availableRamMB;
    double ramUsagePercent = (perfData_.totalRamMB > 0) ? (usedRamMB / perfData_.totalRamMB) * 100.0 : 0.0;
    PrintBar(right_col_x, current_y++, ramUsagePercent, "RAM Usage");

    std::stringstream ram_details;
    ram_details << "\033[0;37m" << std::fixed << std::setprecision(0) << usedRamMB << " MB / " << perfData_.totalRamMB << " MB";
    PrintToBuffer(right_col_x, current_y++, ram_details.str());
    current_y++;

    std::stringstream pf_ss;
    pf_ss << "\033[1;97mPage Faults\033[0;37m: \033[1;36m" << std::fixed << std::setprecision(0) << perfData_.pageFaultsPerSec << "/s";
    PrintToBuffer(right_col_x, current_y, pf_ss.str());

    // 5. Отрисовка
    WriteConsoleOutputA(consoleHandles_[activeBufferIndex_], charBuffer_, bufferSize_, bufferCoord_, &consoleWriteArea_);
    SetConsoleActiveScreenBuffer(consoleHandles_[activeBufferIndex_]);
    activeBufferIndex_ = 1 - activeBufferIndex_;
}

void PerformanceMonitor::Run() {
    while (true) {
        HWND console_wnd = GetConsoleWindow();
        if (console_wnd == NULL || !IsWindowVisible(console_wnd)) {
            break;
        }
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
        std::cerr << "A critical error occurred in Performance Monitor: " << e.what() << std::endl;
        std::cerr << "Press Enter to exit." << std::endl;
        std::cin.get();
        return 1;
    }
    return 0;
}