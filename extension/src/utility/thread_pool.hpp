#pragma once

#include <unordered_map>
#include <vector>
#include <atomic>
#include <mutex>
#include <shared_mutex>
#include <thread>
#include <condition_variable>
#include <functional>

/*
    Thread Pool works by running month tick on every given pointer, or any other work function

*/


template <typename T>
class ThreadPool {
    // T is a pointer, smart or dumb

    std::thread month_tick_checker; //used to check if next day is ready without blocking
    std::mutex month_tick_checker_mutex;
    bool month_tick_flag = false;
    std::condition_variable month_tick_checker_cv;
    std::vector<std::thread> worker_threads;
    mutable std::mutex work_to_process_mutex;
    std::vector<T> work_to_process;
    std::condition_variable condition;
    std::atomic<bool> stop = false;

    std::atomic<int> jobs_remaining = 0;
    std::condition_variable jobs_done_cv;
    std::mutex jobs_done_mutex;

    std::function<void(T)> work_function = [](T object) {
        object->month_tick();
    };

    std::function<void()> work_adder_function;

    void month_tick_check() {
        while (!stop) {
            {
                std::unique_lock<std::mutex> lock(month_tick_checker_mutex); // Waits for month tick
                month_tick_checker_cv.wait(lock, [this] { return stop || month_tick_flag; } );
                month_tick_flag = false;
            }
            TerminalMap::get_instance()->pause_time(); // Pause time while dealing with last months work
            {   
                std::unique_lock<std::mutex> lock(jobs_done_mutex);
                jobs_done_cv.wait(lock, [this] { // Sleeps and waits for jobs to be done 
                    return jobs_remaining == 0; 
                });
            }
            TerminalMap::get_instance()->unpause_time();
            month_tick_helper();
        }
    }

    void month_tick_helper() {
        {
            std::unique_lock lock(work_to_process_mutex);
            work_adder_function();
        }
        
        jobs_remaining  = work_to_process.size();
        condition.notify_all();
    }

    void thread_processor() {
        while (!stop) {
            T to_process = nullptr;

            {
                std::unique_lock<std::mutex> lock(work_to_process_mutex);
                condition.wait(lock, [this]() {
                    return !work_to_process.empty() || stop;
                });

                if (stop && work_to_process.empty()) {
                    return; // Exit thread
                }

                // Get one province to process
                to_process = work_to_process.back();
                work_to_process.pop_back();
            }
            work_function(to_process);

            if (--jobs_remaining == 0) {
                jobs_done_cv.notify_one();  // Wake main thread
            }
        }
    }

    public: 
    ThreadPool(int num_of_threads = 2, std::function<void()> p_work_adder_function = [](){}) {
        work_adder_function = std::move(p_work_adder_function);
        for (int i = 0; i < num_of_threads; i++) {
            worker_threads.emplace_back(&ThreadPool::thread_processor, this);
        }
        month_tick_checker = std::thread(&ThreadPool::month_tick_check, this);
    }

    ~ThreadPool() {
        stop = true;
        month_tick_checker_cv.notify_all();
        condition.notify_all();
    }

    void month_tick() {
        {
            std::scoped_lock lock(month_tick_checker_mutex);
            month_tick_flag = true;
        }
        month_tick_checker_cv.notify_one();
    }

    void set_work_function(std::function<void(T)> func) {
        work_function = std::move(func);
    }

    /// @brief Function should only be used inside of the thread pool helper function to add work to the helper threads
    /// @param work_to_add 
    void add_work(T work_to_add) {
        work_to_process.push_back(work_to_add);
    }
};
