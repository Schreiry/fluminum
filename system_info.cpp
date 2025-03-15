// system_info.cpp
#include "system_info.h"
#include <iostream>
#include <thread>
#include <algorithm> // For std::clamp
#include <cmath>     // For std::log2, std::floor, std::min
#ifdef _WIN32
#include <windows.h>
#include <string>
#include <sstream>
#elif __linux__
#include <fstream>
#include <string>
#include <unistd.h>
#include <sys/sysinfo.h>
#include <stdexcept>
#endif

using std::string;

// ::::::::: Colors ::::::::::
const string RED = "\033[1;31m";
const string GREEN = "\033[1;32m";
const string YELLOW = "\033[1;33m";
const string BLUE = "\033[1;34m";
const string PURPLE = "\033[1;35m";
const string CYAN = "\033[1;36m";
const string RESET = "\033[0m";


std::string cpuName = "Unknown";
int cpuCores = 1;
int cpuThreads = 1;
const int reservedThreadsForOS = 2; // Number of threads to reserve for the OS

void getCPUInfo() {
#ifdef _WIN32
    SYSTEM_INFO sysInfo;
    GetSystemInfo(&sysInfo);
    cpuCores = sysInfo.dwNumberOfProcessors;
    cpuThreads = sysInfo.dwNumberOfProcessors; // On Windows, this often represents logical cores

    char cpuNameBuffer[49];
    HKEY hKey;
    if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, "HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0", 0, KEY_READ, &hKey) == ERROR) {
        cpuName = "Could not retrieve CPU name (Windows)";
    }
    else {
        DWORD bufferSize = sizeof(cpuNameBuffer);
        if (RegQueryValueExA(hKey, "ProcessorNameString", nullptr, nullptr, (LPBYTE)cpuNameBuffer, &bufferSize) == ERROR) {
            cpuName = cpuNameBuffer;
        }
        RegCloseKey(hKey);
    }
#elif __linux__
    std::ifstream cpuinfo("/proc/cpuinfo");
    std::string line;
    while (std::getline(cpuinfo, line)) {
        if (line.substr(0, 10) == "processor\t") {
            cpuThreads++;
        }
        else if (line.substr(0, 10) == "model name\t" && cpuName == "Unknown") {
            cpuName = line.substr(line.find(":") + 2);
        }
        else if (line.substr(0, 6) == "cores\t" && cpuCores == 1) {
            try {
                cpuCores = std::stoi(line.substr(line.find(":") + 2));
            }
            catch (const std::invalid_argument& e) {
                std::cerr << RED << "Error parsing CPU cores: " << e.what() << RESET << std::endl;
            }
            catch (const std::out_of_range& e) {
                std::cerr << RED << "Error parsing CPU cores (out of range): " << e.what() << RESET << std::endl;
            }
        }
    }
    cpuCores = sysconf(_SC_NPROCESSORS_ONLN);
    cpuThreads = sysconf(_SC_NPROCESSORS_CONF);
    cpuinfo.close();
#else
    cpuName = "Operating system not supported for detailed CPU info";
    cpuCores = std::thread::hardware_concurrency();
    cpuThreads = std::thread::hardware_concurrency();
#endif
    if (cpuCores == 0) cpuCores = 1;
    if (cpuThreads == 0) cpuThreads = 1;
}

int calculateOptimalThreads(int matrixSize) {
    int availableThreads = std::thread::hardware_concurrency() - reservedThreadsForOS;
    if (availableThreads < 1) return 1;

    // Formula to calculate suggested number of threads based on matrix size
    // and ensuring it's a power of two and within the available limits.
    double log2Size = std::log2(std::max(1.0, static_cast<double>(matrixSize) / 64.0));
    int exponent = static_cast<int>(std::floor(log2Size));
    int maxExponent = static_cast<int>(std::floor(std::log2(availableThreads)));
    int finalExponent = std::min(maxExponent, exponent);
    int suggestedThreads = 1 << finalExponent;

    int optimalThreads = std::clamp(suggestedThreads, 1, availableThreads);

    std::cout << YELLOW << "Matrix Size: " << matrixSize << "x" << matrixSize << RESET << std::endl;
    std::cout << YELLOW << "Available Threads for Computation: " << availableThreads << RESET << std::endl;
    std::cout << GREEN << "Optimal Threads for this round: " << optimalThreads << RESET << std::endl;

    return optimalThreads;
}