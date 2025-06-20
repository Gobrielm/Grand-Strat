#pragma once

#include "../../classes/terminal.hpp"

#include <vector>
#include <list>
#include <atomic>

#include <mutex>
#include <shared_mutex>
#include <thread>
#include <condition_variable>

class TerminalMap;

class TerminalMapThreadPool {
    private:
    TerminalMap* terminal_map = nullptr;

    std::mutex m;

    // Threads
    std::thread day_queue_thread;
    std::mutex day_tick_mutex;
    std::condition_variable day_tick_cv;
    bool day_tick_requested = false;

    std::vector<std::thread> day_worker_threads;
    std::vector<std::thread> month_worker_threads;
    std::list<Ref<Terminal>> day_tick_work;
    std::vector<Ref<Terminal>> month_tick_work;
    std::condition_variable new_day_work;
    std::condition_variable new_month_work;
    std::atomic<bool> stop = false;
    std::atomic<int> day_jobs = 0;
    std::atomic<int> month_jobs = 0;
    std::condition_variable day_jobs_cv;
    std::condition_variable month_jobs_cv;
    std::mutex day_jobs_done_mutex;
    std::mutex month_jobs_done_mutex;

    void day_tick_helper();
    void month_tick_helper();

    void day_thread_processor();
    void month_thread_processor();


    public:
    TerminalMapThreadPool();
    ~TerminalMapThreadPool();

    void day_tick();
    void month_tick();

};