#pragma once

#include "../terminal_map.hpp"
#include <unordered_map>
#include <vector>
#include <atomic>
#include <mutex>
#include <shared_mutex>
#include <thread>
#include <condition_variable>
#include <functional>


class PopManagerThreadPool {
    friend class PopManager;
    
    std::chrono::time_point<std::chrono::high_resolution_clock> start_time;
    static constexpr int BATCH_SIZE = 4096;
    std::thread month_tick_checker; //used to check if next day is ready without blocking
    std::mutex month_tick_checker_mutex;
    bool month_tick_flag = false;
    std::condition_variable month_tick_checker_cv;
    std::vector<std::thread> worker_threads;
    mutable std::mutex work_to_process_mutex;
    std::condition_variable condition;
    std::atomic<bool> stop = false;

    std::atomic<int> jobs_remaining = 0;
    std::condition_variable jobs_done_cv;
    std::mutex jobs_done_mutex;

    std::vector<int> provinces_to_process;

    std::function<int()> work_adder_function;

    std::function<void(int)> work_function;

    void month_tick_check();

    void month_tick_helper();

    void thread_processor();


    public:

    PopManagerThreadPool(int num_of_threads = 2, std::function<int()> p_work_adder_function = [](){ return 0; });

    ~PopManagerThreadPool();

    void month_tick();

    void set_work_function(std::function<void(int)> func);
};