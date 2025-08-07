
#define NOMINMAX
#include "Algorithm.h"
#include "Matrix.h"
#include "System.h"
#include "IO.h" // For progress bar

// --- Result Struct Constructors ---
MultiplicationResult::MultiplicationResult() :
    resultMatrix(0, 0), durationSeconds_chrono(0.0), durationNanoseconds_chrono(0LL),
    durationSeconds_qpc(0.0), threadsUsed(0), coresDetected(0),
    memoryInfo({ 0 }), strassenThreshold(0),
    originalRowsA(0), originalColsA(0), originalRowsB(0), originalColsB(0) {
}

ComparisonResult::ComparisonResult() :
    matchCount(0LL), durationSeconds_chrono(0.0), durationNanoseconds_chrono(0LL),
    durationSeconds_qpc(0.0), threadsUsed(0), coresDetected(0),
    memoryInfo({ 0 }), comparisonThreshold(0), epsilon(0.0),
    originalRows(0), originalCols(0) {
}


// --- Thread Pool Implementation ---
ThreadPool::ThreadPool(size_t threads) : stop(false) {
    for (size_t i = 0; i < threads; ++i) {
        workers.emplace_back([this] {
            while (true) {
                std::function<void()> task;
                {
                    std::unique_lock<std::mutex> lock(this->queue_mutex);
                    this->condition.wait(lock, [this] { return this->stop.load() || !this->tasks.empty(); });
                    if (this->stop.load() && this->tasks.empty()) return;
                    task = std::move(this->tasks.front());
                    this->tasks.pop();
                }
                task();
            }
            });
    }
}

ThreadPool::~ThreadPool() {
    stop.store(true);
    condition.notify_all();
    for (std::thread& worker : workers) {
        if (worker.joinable()) worker.join();
    }
}

template<class F, class... Args>
auto ThreadPool::enqueue(F&& f, Args&&... args) -> std::future<std::invoke_result_t<F, Args...>> {
    using return_type = std::invoke_result_t<F, Args...>;
    auto task = std::make_shared<std::packaged_task<return_type()>>(
        std::bind(std::forward<F>(f), std::forward<Args>(args)...)
    );
    std::future<return_type> res = task->get_future();
    {
        std::unique_lock<std::mutex> lock(queue_mutex);
        if (stop.load()) throw std::runtime_error("enqueue on stopped ThreadPool");
        tasks.emplace([task]() { (*task)(); });
    }
    condition.notify_one();
    return res;
}
// Explicit template instantiation if needed, or keep in header for templates.

// --- Progress Bar Implementation ---
long long calculate_total_tasks(int n, int threshold) {
    if (n <= 0) return 0;
    int eff_threshold = (threshold == 0) ? 1 : threshold;
    if (n <= eff_threshold) return 1LL;
    return 7LL * calculate_total_tasks(n / 2, threshold) + 1; // +1 for current level
}

void display_progress(std::atomic<int>& counter, long long total, std::atomic<bool>& done) {
    int last_percent = -1;
    auto start_time = std::chrono::steady_clock::now();
    while (!done.load(std::memory_order_acquire)) {
        int current_count = counter.load(std::memory_order_acquire);
        int percent = (total > 0) ? static_cast<int>((static_cast<double>(current_count) / total) * 100.0) : 100;
        percent = std::min(100, percent);
        if (percent > last_percent || percent == 0) {
            cout << "\r" << YELLOW << "Progress: [" << GREEN;
            int bar_width = 30;
            int pos = (bar_width * percent) / 100;
            for (int i = 0; i < bar_width; ++i) cout << (i < pos ? '#' : '-');
            cout << YELLOW << "] " << std::setw(3) << percent << "% (" << current_count << "/" << total << ")" << RESET << std::flush;
            last_percent = percent;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(150));
    }
    auto end_time = std::chrono::steady_clock::now();
    double elapsed_sec = std::chrono::duration<double>(end_time - start_time).count();
    cout << "\r" << YELLOW << "Progress: [" << GREEN << std::string(30, '#') << YELLOW << "] 100% (" << total << "/" << total << ") "
        << GREEN << "Done in " << std::fixed << std::setprecision(2) << elapsed_sec << "s." << RESET << string(15, ' ') << std::flush;
    cout << endl;
}


// --- Strassen Multiplication ---
Matrix strassen_recursive_worker(ThreadPool& pool, Matrix A, Matrix B, int threshold, int current_depth, int max_depth_async, std::atomic<int>& progress_counter);

MultiplicationResult multiplyStrassenParallel(const Matrix& A_orig, const Matrix& B_orig, int threshold, unsigned int num_threads_request) {
    MultiplicationResult result_obj;
    result_obj.originalRowsA = A_orig.rows();
    result_obj.originalColsA = A_orig.cols();
    result_obj.originalRowsB = B_orig.rows();
    result_obj.originalColsB = B_orig.cols();
    result_obj.strassenThreshold = threshold;

    if (A_orig.cols() != B_orig.rows()) throw std::invalid_argument("Matrix dimensions incompatible (A.cols != B.rows).");
    if (A_orig.isEmpty() || B_orig.isEmpty()) {
        result_obj.resultMatrix = Matrix(A_orig.rows(), B_orig.cols());
        result_obj.memoryInfo = getProcessMemoryUsage();
        result_obj.coresDetected = getCpuCoreCount();
        return result_obj;
    }

    unsigned int hardware_cores = getCpuCoreCount();
    result_obj.coresDetected = hardware_cores;
    result_obj.threadsUsed = (num_threads_request == 0) ? hardware_cores : std::min(num_threads_request, hardware_cores);
    if (result_obj.threadsUsed == 0) result_obj.threadsUsed = 1;

    int max_orig_dim = std::max({ A_orig.rows(), A_orig.cols(), B_orig.rows(), B_orig.cols() });
    int padded_size = nextPowerOf2(max_orig_dim);

    auto total_op_start_chrono = std::chrono::high_resolution_clock::now();
    LARGE_INTEGER total_op_start_qpc = { 0 };
    if (g_performanceFrequency.QuadPart != 0) QueryPerformanceCounter(&total_op_start_qpc);

    auto pad_start = std::chrono::high_resolution_clock::now();
    Matrix Apad = Matrix::pad(A_orig, padded_size);
    Matrix Bpad = Matrix::pad(B_orig, padded_size);
    auto pad_end = std::chrono::high_resolution_clock::now();
    result_obj.padding_duration_sec = std::chrono::duration<double>(pad_end - pad_start).count();

    Matrix Cpad(padded_size, padded_size);
    int max_depth_async = (result_obj.threadsUsed > 1) ? static_cast<int>(std::floor(std::log(static_cast<double>(result_obj.threadsUsed)) / std::log(7.0))) : 0;
    if (max_depth_async < 0) max_depth_async = 0;

    std::atomic<int> progress_counter(0);
    std::atomic<bool> multiplication_done(false);
    std::thread progress_thread;
    long long total_tasks = 0;

    result_obj.strassen_applied_at_top_level = (padded_size > threshold && threshold > 0);

    if (result_obj.strassen_applied_at_top_level) {
        total_tasks = calculate_total_tasks(padded_size, threshold);
        print_line_in_box(CYAN + " Starting parallel Strassen..." + RESET, 80, false);
        progress_thread = std::thread(display_progress, std::ref(progress_counter), total_tasks, std::ref(multiplication_done));

        ThreadPool pool(result_obj.threadsUsed);
        Cpad = strassen_recursive_worker(pool, Apad, Bpad, threshold, 0, max_depth_async, std::ref(progress_counter));

    }
    else {
        print_line_in_box(CYAN + " Using Naive multiplication (Size <= Threshold or Threshold=0)..." + RESET, 80, false);
        Cpad = Apad.multiply_naive(Bpad);
    }

    if (progress_thread.joinable()) {
        multiplication_done.store(true, std::memory_order_release);
        progress_thread.join();
    }

    auto unpad_start = std::chrono::high_resolution_clock::now();
    result_obj.resultMatrix = Matrix::unpad(Cpad, A_orig.rows(), B_orig.cols());
    auto unpad_end = std::chrono::high_resolution_clock::now();
    result_obj.unpadding_duration_sec = std::chrono::duration<double>(unpad_end - unpad_start).count();

    auto total_op_end_chrono = std::chrono::high_resolution_clock::now();
    LARGE_INTEGER total_op_end_qpc = { 0 };
    if (g_performanceFrequency.QuadPart != 0) QueryPerformanceCounter(&total_op_end_qpc);

    result_obj.durationSeconds_chrono = std::chrono::duration<double>(total_op_end_chrono - total_op_start_chrono).count();
    if (g_performanceFrequency.QuadPart > 0) {
        result_obj.durationSeconds_qpc = static_cast<double>(total_op_end_qpc.QuadPart - total_op_start_qpc.QuadPart) / g_performanceFrequency.QuadPart;
    }
    result_obj.memoryInfo = getProcessMemoryUsage();
    return result_obj;
}

Matrix strassen_recursive_worker(ThreadPool& pool, Matrix A, Matrix B, int threshold, int current_depth, int max_depth_async, std::atomic<int>& progress_counter) {
    if (A.rows() <= threshold) {
        progress_counter.fetch_add(1, std::memory_order_relaxed);
        return A.multiply_naive(B);
    }

    Matrix A11, A12, A21, A22, B11, B12, B21, B22;
    Matrix::split(A, B, A11, A12, A21, A22, B11, B12, B21, B22);

    Matrix S1 = B12 - B22; Matrix S2 = A11 + A12; Matrix S3 = A21 + A22;
    Matrix S4 = B21 - B11; Matrix S5 = A11 + A22; Matrix S6 = B11 + B22;
    Matrix S7 = A12 - A22; Matrix S8 = B21 + B22; Matrix S9 = A21 - A11;
    Matrix S10 = B11 + B12;

    bool launch_async_here = (current_depth < max_depth_async);

    if (launch_async_here) {
        auto fP1 = pool.enqueue(strassen_recursive_worker, std::ref(pool), S5, S6, threshold, current_depth + 1, max_depth_async, std::ref(progress_counter));
        auto fP2 = pool.enqueue(strassen_recursive_worker, std::ref(pool), S3, B11, threshold, current_depth + 1, max_depth_async, std::ref(progress_counter));
        auto fP3 = pool.enqueue(strassen_recursive_worker, std::ref(pool), A11, S1, threshold, current_depth + 1, max_depth_async, std::ref(progress_counter));
        auto fP4 = pool.enqueue(strassen_recursive_worker, std::ref(pool), A22, S4, threshold, current_depth + 1, max_depth_async, std::ref(progress_counter));
        auto fP5 = pool.enqueue(strassen_recursive_worker, std::ref(pool), S2, B22, threshold, current_depth + 1, max_depth_async, std::ref(progress_counter));
        auto fP6 = pool.enqueue(strassen_recursive_worker, std::ref(pool), S9, S10, threshold, current_depth + 1, max_depth_async, std::ref(progress_counter));
        auto fP7 = pool.enqueue(strassen_recursive_worker, std::ref(pool), S7, S8, threshold, current_depth + 1, max_depth_async, std::ref(progress_counter));

        Matrix P1 = fP1.get(); Matrix P2 = fP2.get(); Matrix P3 = fP3.get(); Matrix P4 = fP4.get();
        Matrix P5 = fP5.get(); Matrix P6 = fP6.get(); Matrix P7 = fP7.get();

        Matrix C11 = P1 + P4 - P5 + P7; Matrix C12 = P3 + P5;
        Matrix C21 = P2 + P4;           Matrix C22 = P1 - P2 + P3 + P6;
        progress_counter.fetch_add(1, std::memory_order_relaxed);
        return Matrix::combine(C11, C12, C21, C22);

    }
    else {
        Matrix P1 = strassen_recursive_worker(pool, S5, S6, threshold, current_depth + 1, max_depth_async, progress_counter);
        Matrix P2 = strassen_recursive_worker(pool, S3, B11, threshold, current_depth + 1, max_depth_async, progress_counter);
        Matrix P3 = strassen_recursive_worker(pool, A11, S1, threshold, current_depth + 1, max_depth_async, progress_counter);
        Matrix P4 = strassen_recursive_worker(pool, A22, S4, threshold, current_depth + 1, max_depth_async, progress_counter);
        Matrix P5 = strassen_recursive_worker(pool, S2, B22, threshold, current_depth + 1, max_depth_async, progress_counter);
        Matrix P6 = strassen_recursive_worker(pool, S9, S10, threshold, current_depth + 1, max_depth_async, progress_counter);
        Matrix P7 = strassen_recursive_worker(pool, S7, S8, threshold, current_depth + 1, max_depth_async, progress_counter);

        Matrix C11 = P1 + P4 - P5 + P7; Matrix C12 = P3 + P5;
        Matrix C21 = P2 + P4;           Matrix C22 = P1 - P2 + P3 + P6;
        progress_counter.fetch_add(1, std::memory_order_relaxed);
        return Matrix::combine(C11, C12, C21, C22);
    }
}


// --- Parallel Matrix Comparison ---
long long compareMatricesInternal(ThreadPool& pool, const Matrix& A_rec, const Matrix& B_rec, int threshold, double epsilon, int current_depth, int max_depth_async_comp);

ComparisonResult compareMatricesParallel(const Matrix& A_orig, const Matrix& B_orig, int threshold, double epsilon, unsigned int num_threads_request) {
    ComparisonResult result_obj;
    result_obj.originalRows = A_orig.rows();
    result_obj.originalCols = A_orig.cols();
    result_obj.comparisonThreshold = threshold;
    result_obj.epsilon = epsilon;

    if (A_orig.rows() != B_orig.rows() || A_orig.cols() != B_orig.cols()) {
        throw std::invalid_argument("Matrix dimensions must be identical for comparison.");
    }
    if (A_orig.isEmpty()) {
        result_obj.memoryInfo = getProcessMemoryUsage();
        result_obj.coresDetected = getCpuCoreCount();
        result_obj.matchCount = 0;
        return result_obj;
    }

    unsigned int hardware_cores = getCpuCoreCount();
    result_obj.coresDetected = hardware_cores;
    result_obj.threadsUsed = (num_threads_request == 0) ? hardware_cores : std::min(num_threads_request, hardware_cores);
    if (result_obj.threadsUsed == 0) result_obj.threadsUsed = 1;

    ThreadPool pool(result_obj.threadsUsed);
    int max_depth_async_comp = (result_obj.threadsUsed > 1) ? static_cast<int>(std::floor(std::log(static_cast<double>(result_obj.threadsUsed)) / std::log(4.0))) : 0;
    if (max_depth_async_comp < 0) max_depth_async_comp = 0;

    auto start_time_chrono = std::chrono::high_resolution_clock::now();
    LARGE_INTEGER start_time_qpc = { 0 };
    if (g_performanceFrequency.QuadPart != 0) QueryPerformanceCounter(&start_time_qpc);

    // Padding to keep the recursive structure simple.
    int max_orig_dim = std::max(A_orig.rows(), A_orig.cols());
    int padded_size = nextPowerOf2(max_orig_dim);
    Matrix Apad = Matrix::pad(A_orig, padded_size);
    Matrix Bpad = Matrix::pad(B_orig, padded_size);

    long long total_matches_padded = compareMatricesInternal(pool, Apad, Bpad, threshold, epsilon, 0, max_depth_async_comp);

    // To get the correct count, we must subtract matches from the padded area.
    long long total_elements_padded = (long long)padded_size * padded_size;
    long long total_elements_orig = (long long)A_orig.rows() * A_orig.cols();
    long long padded_matches = (total_elements_padded - total_elements_orig); // In padding, 0==0
    result_obj.matchCount = total_matches_padded - padded_matches;


    auto end_time_chrono = std::chrono::high_resolution_clock::now();
    LARGE_INTEGER end_time_qpc = { 0 };
    if (g_performanceFrequency.QuadPart != 0) QueryPerformanceCounter(&end_time_qpc);

    result_obj.durationSeconds_chrono = std::chrono::duration<double>(end_time_chrono - start_time_chrono).count();
    if (g_performanceFrequency.QuadPart > 0) {
        result_obj.durationSeconds_qpc = static_cast<double>(end_time_qpc.QuadPart - start_time_qpc.QuadPart) / g_performanceFrequency.QuadPart;
    }
    result_obj.memoryInfo = getProcessMemoryUsage();
    return result_obj;
}

long long compareMatricesInternal(ThreadPool& pool, const Matrix& A_rec, const Matrix& B_rec, int threshold, double epsilon, int current_depth, int max_depth_async_comp) {
    if (A_rec.rows() <= threshold || A_rec.isEmpty()) {
        return A_rec.compare_naive(B_rec, epsilon);
    }

    Matrix A11, A12, A21, A22, B11, B12, B21, B22;
    Matrix::split(A_rec, B_rec, A11, A12, A21, A22, B11, B12, B21, B22);

    bool launch_async_here = (current_depth < max_depth_async_comp);

    if (launch_async_here) {
        auto f_c11 = pool.enqueue(compareMatricesInternal, std::ref(pool), std::cref(A11), std::cref(B11), threshold, epsilon, current_depth + 1, max_depth_async_comp);
        auto f_c12 = pool.enqueue(compareMatricesInternal, std::ref(pool), std::cref(A12), std::cref(B12), threshold, epsilon, current_depth + 1, max_depth_async_comp);
        auto f_c21 = pool.enqueue(compareMatricesInternal, std::ref(pool), std::cref(A21), std::cref(B21), threshold, epsilon, current_depth + 1, max_depth_async_comp);
        auto f_c22 = pool.enqueue(compareMatricesInternal, std::ref(pool), std::cref(A22), std::cref(B22), threshold, epsilon, current_depth + 1, max_depth_async_comp);
        return f_c11.get() + f_c12.get() + f_c21.get() + f_c22.get();
    }
    else {
        return compareMatricesInternal(pool, A11, B11, threshold, epsilon, current_depth + 1, max_depth_async_comp) +
            compareMatricesInternal(pool, A12, B12, threshold, epsilon, current_depth + 1, max_depth_async_comp) +
            compareMatricesInternal(pool, A21, B21, threshold, epsilon, current_depth + 1, max_depth_async_comp) +
            compareMatricesInternal(pool, A22, B22, threshold, epsilon, current_depth + 1, max_depth_async_comp);
    }
}