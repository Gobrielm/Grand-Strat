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

    std::unordered_map<int, std::vector<BasePop*>> work_to_process;

    std::function<int()> work_adder_function;

    std::function<void(std::vector<BasePop*>&)> work_function = [](std::vector<BasePop*>& v) {
        for (auto& obj: v) {
            obj->month_tick();
        }
    };

    void month_tick_check() {
        while (!stop) {
            {
                std::unique_lock<std::mutex> lock(month_tick_checker_mutex); // Waits for month tick
                month_tick_checker_cv.wait(lock, [this] { return stop || month_tick_flag; } );
                month_tick_flag = false;
            }
            if (stop) break;
            TerminalMap::get_instance()->pause_time(); // Pause time while dealing with last months work
            {   
                std::unique_lock<std::mutex> lock(jobs_done_mutex);
                jobs_done_cv.wait(lock, [this] { // Sleeps and waits for jobs to be done 
                    return jobs_remaining == 0; 
                });
            }
            TerminalMap::get_instance()->unpause_time();
            if (stop) break;
            start_time = std::chrono::high_resolution_clock::now();
            month_tick_helper();
        }
    }

    void month_tick_helper() {
        int new_jobs = 0;
        {
            std::unique_lock lock(work_to_process_mutex);
            new_jobs = work_adder_function();
        }
        
        jobs_remaining  = new_jobs;
        condition.notify_all();
    }

    void thread_processor() {
        while (!stop) {
            std::vector<BasePop*> batch_to_process;
            {
                std::unique_lock<std::mutex> lock(work_to_process_mutex);
                condition.wait(lock, [this]() {
                    return !work_to_process.empty() || stop;
                });

                if (stop && work_to_process.empty()) {
                    return; // Exit thread
                }

                // Get one province to process
                auto &temp_v = work_to_process.begin()->second;
                std::swap(batch_to_process, temp_v);
                if (temp_v.size() == 0) {
                    work_to_process.erase(work_to_process.begin());
                }
            }
            int size_processed = batch_to_process.size();
            work_function(batch_to_process);
            jobs_remaining -= size_processed;

            if (jobs_remaining == 0) {
                std::chrono::duration<double> elapsed = std::chrono::high_resolution_clock::now() - start_time;
                if (elapsed.count() > 15) {
                    print_line("Pop Manager Month Tick took " + String::num_scientific(elapsed.count()) + " seconds");
                }
                jobs_done_cv.notify_one();  // Wake main thread
            }
        }
    }


    public:

    PopManagerThreadPool(int num_of_threads = 2, std::function<int()> p_work_adder_function = [](){ return 0; }) {
        work_adder_function = std::move(p_work_adder_function);
        for (int i = 0; i < num_of_threads; i++) {
            worker_threads.emplace_back(&PopManagerThreadPool::thread_processor, this);
        }
        month_tick_checker = std::thread(&PopManagerThreadPool::month_tick_check, this);
    }

    ~PopManagerThreadPool() {
        stop = true;
        month_tick_checker_cv.notify_all();
        condition.notify_all();
        jobs_done_cv.notify_all();

        for (auto &t : worker_threads) {
            if (t.joinable()) {
                t.join();
            }
        }

        if (month_tick_checker.joinable()) {
            month_tick_checker.join();
        }
    }

    void month_tick() {
        {
            std::scoped_lock lock(month_tick_checker_mutex);
            month_tick_flag = true;
        }
        month_tick_checker_cv.notify_one();
    }

    void add_work(BasePop* work_to_add, int mutex_number) {
        work_to_process[mutex_number].push_back(work_to_add);
    }

    void set_work_function(std::function<void(std::vector<BasePop*>&)> func) {
        work_function = std::move(func);
    }
};