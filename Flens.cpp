// main.cpp
#include <iostream>
#include <vector>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <chrono>
#include <atomic>
#include <sstream>
#include <stdexcept>
#include <future>
#include <cmath>
#include <iomanip>

#include "system_info.h"
#include "matrix_operations.h"

// /// /// /// /// /// ///

using std::string;
using std::vector;
using std::thread;
using std::mutex;
using std::unique_lock;
using std::lock_guard;
using std::condition_variable;
using std::chrono::high_resolution_clock;
using std::chrono::duration_cast;
using std::chrono::seconds;
using std::chrono::milliseconds;
using std::stringstream;
using std::atomic;
using std::ref;
using std::future;
using std::launch;
using std::cout;
using std::endl;
using std::setw;
using std::setfill;

// /// /// /// /// /// ///

// ::::::::: Colors ::::::::::
const string RED = "\033[1;31m";
const string GREEN = "\033[1;32m";
const string YELLOW = "\033[1;33m";
const string BLUE = "\033[1;34m";
const string PURPLE = "\033[1;35m";
const string CYAN = "\033[1;36m";
const string RESET = "\033[0m";

// ===== Global variables =====
int currentN = 64; // Start with a size suitable for Strassen
vector<vector<int>> globalA;
vector<vector<int>> globalB;
vector<vector<int>> globalC;

// ===== Mutexes and condition variables =====
mutex mtx;
condition_variable cv;
bool matricesReady = false; // Flag: new matrices are ready for multiplication
atomic<bool> calculationComplete(false);
int numWorkerThreads = 0;

// ===== Function for worker thread =====
void multiplicationWorker(int threadId, int startRow, int endRow) {
    try {
        stringstream ss;
        ss << BLUE << "Thread " << threadId << RESET << ": Processing rows " << startRow << " to " << endRow - 1 << endl;
        cout << ss.str();
        // Note: Strassen's algorithm is applied to the entire matrix, not row by row in this implementation.
        // The row-based threading will need to be adapted or removed if we want to parallelize Strassen directly.
        // For now, the main thread will handle the Strassen multiplication.
        stringstream ss_finish;
        ss_finish << GREEN << "Thread " << threadId << RESET << ": Finished (Strassen is main thread for now)" << endl;
        cout << ss_finish.str();
    }
    catch (const std::exception& e) {
        std::cerr << RED << "Error in thread " << threadId << ": " << e.what() << RESET << endl;
    }
}

// ===== Timer Thread Function =====
void timerThreadFunction(atomic<bool>& finished, std::chrono::steady_clock::time_point startTime) {
    while (!finished.load()) {
        std::this_thread::sleep_for(milliseconds(100)); // Update more frequently for smoother stopwatch
        auto now = high_resolution_clock::now();
        auto elapsed = duration_cast<milliseconds>(now - startTime).count();
        long long seconds = elapsed / 1000;
        long long ms = elapsed % 1000;
        cout << YELLOW << "\rTimer: " << seconds << "." << setw(3) << setfill('0') << ms << " seconds" << RESET << std::flush;
    }
    auto now = high_resolution_clock::now();
    auto elapsed = duration_cast<milliseconds>(now - startTime).count();
    long long seconds = elapsed / 1000;
    long long ms = elapsed % 1000;
    cout << YELLOW << "\rTimer finished: " << seconds << "." << setw(3) << setfill('0') << ms << " seconds" << RESET << endl;
}

int main() {
    cout << PURPLE << "Starting Thread-Matrix Enhanced with Strassen..." << RESET << endl;

    // Analyze system information
    cout << YELLOW << "Analyzing system..." << RESET << endl;
    getCPUInfo();
    cout << CYAN << "CPU Name: " << cpuName << RESET << endl;
    cout << CYAN << "CPU Cores: " << cpuCores << RESET << endl;
    cout << CYAN << "CPU Threads (Logical): " << cpuThreads << RESET << endl;

    // Main loop: generate matrices, multiply, print results and increase size
    while (true) {
        cout << GREEN << "\nGenerator: Creating matrices of size " << currentN << "x" << currentN << RESET << endl;

        // Ensure matrix size is a power of 2 for Strassen's algorithm
        int n = currentN;
        if ((n & (n - 1)) != 0) {
            int nextPowerOfTwo = std::pow(2, std::ceil(std::log2(n)));
            cout << YELLOW << "Warning: Matrix size " << n << " is not a power of 2. Padding to " << nextPowerOfTwo << " for Strassen." << RESET << endl;
            currentN = nextPowerOfTwo;
        }

        // ===== Matrix generation =====
        {
            lock_guard<mutex> lock(mtx);
            globalA.assign(currentN, vector<int>(currentN, 1));
            globalB.assign(currentN, vector<int>(currentN, 2));
            globalC.assign(currentN, vector<int>(currentN, 0));
            matricesReady = true;
            calculationComplete.store(false);
        }
        cv.notify_all();

        // ===== Calculate Optimal Threads based on Matrix Size =====
        numWorkerThreads = calculateOptimalThreads(currentN);
        cout << GREEN << "Number of worker threads for this round: " << numWorkerThreads << RESET << endl;

        // ===== Start the timer thread =====
        atomic<bool> roundFinished(false);
        auto startTime = high_resolution_clock::now();
        thread timerThread(timerThreadFunction, ref(roundFinished), startTime);

        // ===== Perform Strassen's multiplication in the main thread for now =====
        auto multiplyStart = high_resolution_clock::now();
        multiplyMatricesStrassen(globalA, globalB, globalC);
        auto multiplyEnd = high_resolution_clock::now();
        std::chrono::duration<double> multiplyElapsed = multiplyEnd - multiplyStart;
        cout << BLUE << "Strassen Multiplication Time: " << multiplyElapsed.count() << " seconds" << RESET << endl;

        calculationComplete.store(true);

        // ===== Stop the timer thread =====
        roundFinished.store(true);
        timerThread.join();

        // ===== Calculate total execution time =====
        auto endTime = high_resolution_clock::now();
        std::chrono::duration<double> elapsed = endTime - startTime;

        // ===== Output the final result =====
        long long inputElements = static_cast<long long>(currentN) * currentN * 2;
        long long outputElements = static_cast<long long>(currentN) * currentN;
        cout << PURPLE << "\n=============================================" << RESET << endl;
        cout << CYAN << "Calculation complete for matrices of size " << currentN << "x" << currentN << " using Strassen!" << RESET << endl;
        cout << CYAN << "Number of worker threads (based on matrix size): " << numWorkerThreads << RESET << endl;
        cout << CYAN << "Input elements: " << inputElements << ", Output elements: " << outputElements << RESET << endl;
        cout << CYAN << "Total Computation time: " << elapsed.count() << " seconds" << RESET << endl;
        cout << PURPLE << "=============================================\n" << RESET << endl;

        // ===== Increase the size of matrices =====
        currentN *= 2;

        // Small delay
        std::this_thread::sleep_for(seconds(2));
    }

    return 0;
}