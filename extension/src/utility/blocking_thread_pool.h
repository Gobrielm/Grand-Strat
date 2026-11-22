#pragma once

#include <vector>
#include <atomic>
#include <mutex>
#include <shared_mutex>
#include <thread>
#include <condition_variable>
#include <functional>


class BlockingThreadPool {

    int workers_active = 0;
    std::vector<std::thread> worker_threads;
    std::atomic<bool> stop = false;

    std::vector<int> provinces_to_process;
    std::function<std::vector<int>()> work_adder_function;

    std::function<void(int)> work_function;

    void thread_processor();


    public:

    std::condition_variable isWorkAvailable;
    std::condition_variable areJobsDone;
    mutable std::mutex mutex;

    BlockingThreadPool(int num_of_threads = 2, std::function<std::vector<int>()> p_work_adder_function = [](){ return std::vector<int>(); });

    ~BlockingThreadPool();

    // Will Block until everything finishes, returns will time taken
    double month_tick();

    void set_work_function(std::function<void(int)> func);
};