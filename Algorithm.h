#pragma once
#include "Common.h"

// --- Thread Pool ---
class ThreadPool {
public:
    ThreadPool(size_t threads);
    ~ThreadPool();

    template<class F, class... Args>
    auto enqueue(F&& f, Args&&... args) -> std::future<std::invoke_result_t<F, Args...>>;

private:
    std::vector<std::thread> workers;
    std::queue<std::function<void()>> tasks;
    std::mutex queue_mutex;
    std::condition_variable condition;
    std::atomic<bool> stop;
};

// --- Core Algorithms ---

// Strassen's Algorithm, now with a flag to enable tiling for its base cases.
MultiplicationResult multiplyStrassenParallel(const Matrix& A_orig, const Matrix& B_orig, int threshold,
    bool use_tiling_for_base, int tile_size_for_base,
    unsigned int num_threads_request = 0);

// NEW: A standalone, fully parallelized tiled multiplication algorithm.
MultiplicationResult multiplyTiledParallel(const Matrix& A, const Matrix& B, int tileSize,
    unsigned int num_threads_request);


ComparisonResult compareMatricesParallel(const Matrix& A_orig, const Matrix& B_orig, int threshold, double epsilon,
    unsigned int num_threads_request = 0);