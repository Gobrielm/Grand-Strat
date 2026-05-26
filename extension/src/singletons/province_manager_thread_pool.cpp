#include "province_manager_thread_pool.hpp"

#include "singletons/terminal_map.hpp"
#include "singletons/province_manager.hpp"

void ProvinceManagerThreadPool::thread_processor() {
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


ProvinceManagerThreadPool::ProvinceManagerThreadPool(int num_of_threads, std::function<std::vector<int>()> p_work_adder_function) {
    work_adder_function = std::move(p_work_adder_function);
    for (int i = 0; i < num_of_threads; i++) {
        worker_threads.emplace_back(&ProvinceManagerThreadPool::thread_processor, this);
    }
}

ProvinceManagerThreadPool::~ProvinceManagerThreadPool() {
    stop = true;
    isWorkAvailable.notify_all();
    areJobsDone.notify_all();

    for (auto &t : worker_threads) {
        if (t.joinable()) {
            t.join();
        }
    }
}

void ProvinceManagerThreadPool::month_tick() {

    auto start_time = std::chrono::high_resolution_clock::now();
    
    std::unique_lock lock(mutex);

    provinces_to_process = work_adder_function();
    isWorkAvailable.notify_all();
    areJobsDone.wait(lock, [this] { return stop || (provinces_to_process.empty() && workers_active == 0); } );
    
    lock.unlock();

    std::chrono::duration<double> elapsed = std::chrono::high_resolution_clock::now() - start_time;
    if (elapsed.count() > 15) {
        print_line("Province Manager Month Tick took " + String::num_scientific(elapsed.count()) + " seconds");
    }
}

void ProvinceManagerThreadPool::set_work_function(std::function<void(int)> func) {
    work_function = std::move(func);
}