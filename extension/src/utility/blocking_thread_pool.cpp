#include "blocking_thread_pool.h"

#include "singletons/terminal_map.hpp"
#include "singletons/province_manager.hpp"

void BlockingThreadPool::thread_processor() {
    while (!stop) {
        
        std::unique_lock<std::mutex> lock(mutex);
        isWorkAvailable.wait(lock, [this]() {
            return !provinces_to_process.empty() || stop;
        });

        if (stop) {
            return; // Exit thread
        }

        // Get one province to process
        int province_to_process = provinces_to_process.back();
        provinces_to_process.pop_back();
        workers_active++;
        lock.unlock();

        work_function(province_to_process);

        lock.lock();
        workers_active--;

        if (provinces_to_process.empty() && workers_active == 0) {
            areJobsDone.notify_all();  // Wake main thread
        }
        lock.unlock();
    }
}


BlockingThreadPool::BlockingThreadPool(int num_of_threads, std::function<std::vector<int>()> p_work_adder_function) {
    work_adder_function = std::move(p_work_adder_function);
    for (int i = 0; i < num_of_threads; i++) {
        worker_threads.emplace_back(&BlockingThreadPool::thread_processor, this);
    }
}

BlockingThreadPool::~BlockingThreadPool() {
    stop = true;
    isWorkAvailable.notify_all();
    areJobsDone.notify_all();

    for (auto &t : worker_threads) {
        if (t.joinable()) {
            t.join();
        }
    }
}

double BlockingThreadPool::month_tick() {

    auto start_time = std::chrono::high_resolution_clock::now();
    
    std::unique_lock lock(mutex);
    provinces_to_process = work_adder_function();
    isWorkAvailable.notify_all();
    
    areJobsDone.wait(lock, [this] { return stop || (provinces_to_process.empty() && workers_active == 0); } );
    
    lock.unlock();

    std::chrono::duration<double> elapsed = std::chrono::high_resolution_clock::now() - start_time;
    return elapsed.count();
}

void BlockingThreadPool::set_work_function(std::function<void(int)> func) {
    work_function = std::move(func);
}