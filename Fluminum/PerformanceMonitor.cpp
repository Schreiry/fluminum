#include "PerformanceMonitor.h"
#include <iostream>
#include <iomanip>
#include <thread>
#include <stdexcept>
#include <algorithm>
#include <sstream>
#include <chrono>
#include <PdhMsg.h>

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
    GRAY = 7, WHITE = 15
};

// --- Реализация класса PerformanceMonitor ---

PerformanceMonitor::PerformanceMonitor()
    : activeBufferIndex_(0), charBuffer_(nullptr), queryHandle_(nullptr), bufferCoord_({ 0, 0 })
{
    InitConsole();
    QueryStaticInfo();
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
    // Восстанавливаем оригинальный буфер и курсор
    SetConsoleActiveScreenBuffer(consoleHandles_[1]);
    CONSOLE_CURSOR_INFO cursorInfo;
    GetConsoleCursorInfo(consoleHandles_[0], &cursorInfo);
    cursorInfo.bVisible = TRUE;
    SetConsoleCursorInfo(consoleHandles_[0], &cursorInfo);
}

void PerformanceMonitor::InitConsole() {
    SetConsoleTitle(L"Performance Monitor");
    SetConsoleOutputCP(CP_UTF8); // Для корректного отображения символов

    consoleHandles_[1] = GetStdHandle(STD_OUTPUT_HANDLE);
    consoleHandles_[0] = CreateConsoleScreenBuffer(
        GENERIC_READ | GENERIC_WRITE,
        FILE_SHARE_READ | FILE_SHARE_WRITE,
        NULL,
        CONSOLE_TEXTMODE_BUFFER,
        NULL
    );

    if (consoleHandles_[0] == INVALID_HANDLE_VALUE || consoleHandles_[1] == INVALID_HANDLE_VALUE) {
        throw std::runtime_error("Failed to create console screen buffers.");
    }

    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(consoleHandles_[1], &csbi);

    bufferSize_ = { static_cast<SHORT>(csbi.srWindow.Right - csbi.srWindow.Left + 1), static_cast<SHORT>(csbi.srWindow.Bottom - csbi.srWindow.Top + 1) };
    if (bufferSize_.X <= 0 || bufferSize_.Y <= 0) {
        // Установка размера по умолчанию, если окно свернуто
        bufferSize_ = { 120, 30 };
    }

    // Устанавливаем размер буфера в соответствии с размером окна
    SetConsoleScreenBufferSize(consoleHandles_[0], bufferSize_);

    charBuffer_ = new CHAR_INFO[bufferSize_.X * bufferSize_.Y];
    consoleWriteArea_ = { 0, 0, static_cast<SHORT>(bufferSize_.X - 1), static_cast<SHORT>(bufferSize_.Y - 1) };

    CONSOLE_CURSOR_INFO cursorInfo;
    GetConsoleCursorInfo(consoleHandles_[0], &cursorInfo);
    cursorInfo.bVisible = FALSE; // Скрываем курсор
    SetConsoleCursorInfo(consoleHandles_[0], &cursorInfo);
    SetConsoleCursorInfo(consoleHandles_[1], &cursorInfo);
}

void PerformanceMonitor::QueryStaticInfo() {
    // Получение имени процессора
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

    // Количество логических ядер
    SYSTEM_INFO sysInfo;
    GetSystemInfo(&sysInfo);
    staticInfo_.logicalCoreCount = sysInfo.dwNumberOfProcessors;

    // Получение информации о кэше
    DWORD bufferSize = 0;
    GetLogicalProcessorInformationEx(RelationCache, nullptr, &bufferSize);
    if (bufferSize > 0) {
        std::vector<char> buffer(bufferSize);
        PSYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX procInfo = reinterpret_cast<PSYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX>(buffer.data());
        if (GetLogicalProcessorInformationEx(RelationCache, procInfo, &bufferSize)) {
            char* ptr = buffer.data();
            while (ptr < buffer.data() + bufferSize) {
                PSYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX current = reinterpret_cast<PSYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX>(ptr);
                if (current->Relationship == RelationCache && current->Cache.Level != 0) {
                    staticInfo_.caches.push_back({
                        current->Cache.Level,
                        current->Cache.CacheSize,
                        current->Cache.LineSize,
                        current->Cache.Associativity
                        });
                }
                ptr += current->Size;
            }
        }
    }

    // Получение общего объема RAM
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

    // ИСПРАВЛЕНО: Использование PdhAddEnglishCounterW для независимости от языка ОС
    // и добавлена проверка статуса для каждого счётчика.
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

    // "Прогревочный" сбор данных для инициализации счетчиков
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
        if (status == PDH_INVALID_DATA) {
            perfData_.totalCpuUsage = 0.0;
        }
        else {
            std::stringstream err;
            err << "PDH collect error: 0x" << std::hex << status;
            PrintToBuffer(2, bufferSize_.Y - 2, "\033[1;31m" + err.str());
            if (!RestartPdhQuery()) {
                PrintToBuffer(2, bufferSize_.Y - 1, "\033[1;31mFATAL: Query restart failed.");
            }
        }
        return;
    }

    PDH_FMT_COUNTERVALUE value;

    // CPU Total
    status = PdhGetFormattedCounterValue(totalCpuCounter_, PDH_FMT_DOUBLE, NULL, &value);
    perfData_.totalCpuUsage = (status == ERROR_SUCCESS) ? value.doubleValue : 0.0;

    // CPU Cores
    for (int i = 0; i < staticInfo_.logicalCoreCount; ++i) {
        status = PdhGetFormattedCounterValue(coreCounters_[i], PDH_FMT_DOUBLE, NULL, &value);
        perfData_.coreUsage[i] = (status == ERROR_SUCCESS) ? value.doubleValue : 0.0;
    }

    // RAM
    status = PdhGetFormattedCounterValue(availableMemoryCounter_, PDH_FMT_LARGE, NULL, &value);
    perfData_.availableRamMB = (status == ERROR_SUCCESS) ? value.largeValue : 0;

    // Page Faults
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
                else if (code == "[0")    attribute = GRAY;
                else if (code == "[1;97") attribute = WHITE;
            }
            else {
                code += c;
            }
        }
        else {
            if (current_x >= 0) {
                charBuffer_[y * bufferSize_.X + current_x].Char.AsciiChar = c;
                charBuffer_[y * bufferSize_.X + current_x].Attributes = attribute;
            }
            current_x++;
        }
    }
}

void PerformanceMonitor::PrintBar(int x, int y, int width, double percentage, const std::string& label) {
    std::stringstream ss;
    ss << std::left << std::setw(12) << label << " \033[0;37m[\033[1;32m";

    double clamped_val = percentage;
    if (clamped_val < 0.0) clamped_val = 0.0;
    if (clamped_val > 100.0) clamped_val = 100.0;

    int bar_fill = static_cast<int>((clamped_val / 100.0) * width);

    if (clamped_val >= 75) {
        ss << "\033[1;31m";
    }
    else if (clamped_val >= 40) {
        ss << "\033[1;33m";
    }

    for (int i = 0; i < width; ++i) {
        ss << (i < bar_fill ? '\xDB' : '\xB1');
    }
    ss << "\033[0;37m] \033[1;36m" << std::fixed << std::setprecision(1) << std::setw(5) << std::right << clamped_val << "%";
    PrintToBuffer(x, y, ss.str());
}

void PerformanceMonitor::Render() {
    for (int i = 0; i < bufferSize_.X * bufferSize_.Y; ++i) {
        charBuffer_[i].Char.AsciiChar = ' ';
        charBuffer_[i].Attributes = GRAY;
    }

    PrintToBuffer(2, 1, "\033[1;35m--- FLUMINUM PERFORMANCE MONITOR ---");
    PrintToBuffer(2, 3, "\033[1;33mProcessor: \033[1;97m" + staticInfo_.cpuName);

    std::stringstream cache_ss;
    cache_ss << "\033[1;33mCaches: \033[1;97m";
    for (const auto& cache : staticInfo_.caches) {
        cache_ss << "L" << cache.level << ": " << cache.size / 1024 << "K  ";
    }
    PrintToBuffer(2, 4, cache_ss.str());

    int y_pos = 6;
    PrintToBuffer(2, y_pos, "\033[1;32m--- CPU Usage ---");
    y_pos += 2;
    PrintBar(2, y_pos++, 40, perfData_.totalCpuUsage, "CPU Total");
    for (int i = 0; i < staticInfo_.logicalCoreCount; ++i) {
        if (y_pos >= bufferSize_.Y - 4) break; // Оставим место для сообщений об ошибках
        PrintBar(4, y_pos++, 40, perfData_.coreUsage[i], "Core " + std::to_string(i));
    }

    int mem_x_pos = 65;
    int mem_y_pos = 6;

    PrintToBuffer(mem_x_pos, mem_y_pos++, "\033[1;32m--- Memory ---");
    mem_y_pos++;

    double usedRamMB = perfData_.totalRamMB - perfData_.availableRamMB;
    double ramUsagePercent = (perfData_.totalRamMB > 0) ? (usedRamMB / perfData_.totalRamMB) * 100.0 : 0.0;

    std::stringstream ram_ss;
    ram_ss << std::fixed << std::setprecision(0) << usedRamMB << " / " << perfData_.totalRamMB << " MB";
    PrintBar(mem_x_pos, mem_y_pos++, 40, ramUsagePercent, "RAM Usage");
    PrintToBuffer(mem_x_pos + 14, mem_y_pos++, "\033[0;37m" + ram_ss.str());
    mem_y_pos++;

    std::stringstream pf_ss;
    pf_ss << "\033[1;33mPage Faults/sec: \033[1;36m" << std::fixed << std::setprecision(0) << perfData_.pageFaultsPerSec;
    PrintToBuffer(mem_x_pos, mem_y_pos, pf_ss.str());

    WriteConsoleOutputA(consoleHandles_[activeBufferIndex_], charBuffer_, bufferSize_, bufferCoord_, &consoleWriteArea_);
    SetConsoleActiveScreenBuffer(consoleHandles_[activeBufferIndex_]);

    activeBufferIndex_ = 1 - activeBufferIndex_;
}

void PerformanceMonitor::Run() {
    // Бесконечный цикл для отдельного процесса.
    // Выход произойдет при закрытии окна консоли.
    while (true) {
        // Проверяем, видимо ли еще окно консоли. Если пользователь его закрыл, выходим.
        HWND console_wnd = GetConsoleWindow();
        if (console_wnd == NULL || !IsWindowVisible(console_wnd)) {
            break;
        }

        CollectDynamicData();
        Render();
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }
}

// --- Точка входа для процесса мониторинга ---
int RunPerformanceMonitorEntry() {
    try {
        PerformanceMonitor monitor;
        monitor.Run();
    }
    catch (const std::exception& e) {
        // Выводим ошибку в консоль и ждем, чтобы пользователь мог ее прочитать
        std::cerr << "A critical error occurred in Performance Monitor: " << e.what() << std::endl;
        std::cerr << "Press Enter to exit." << std::endl;
        std::cin.get();
        return 1;
    }
    return 0;
}