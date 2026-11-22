#include "pop_manager_thread_pool.hpp"
#include "singletons/pop_manager.hpp"

void PopManagerThreadPool::month_tick_check() {
    while (!stop) {
        {
            std::unique_lock<std::mutex> lock(month_tick_checker_mutex); // Waits for month tick
            month_tick_checker_cv.wait(lock, [this] { return stop || month_tick_flag; } );
            month_tick_flag = false;
        }

        if (stop) break;
        TerminalMap::get_instance()->pause_time(); // Pause time while dealing with last months work
        {   
            std::unique_lock<std::mutex> lock(work_to_process_mutex);
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

void PopManagerThreadPool::month_tick_helper() {
    std::unique_lock lock(work_to_process_mutex);
    jobs_remaining = work_adder_function();
    condition.notify_all();
}

void PopManagerThreadPool::thread_processor() {
    while (!stop) {
        int province_to_process;
        
        std::unique_lock<std::mutex> lock(work_to_process_mutex);
        condition.wait(lock, [this]() {
            return !provinces_to_process.empty() || stop;
        });

        if (stop) {
            return; // Exit thread
        }

        // Get one province to process
        province_to_process = provinces_to_process.back();
        provinces_to_process.pop_back();
        lock.unlock();

        work_function(province_to_process);

        lock.lock();
        jobs_remaining -= 1;
        if (jobs_remaining == 0) {
            std::chrono::duration<double> elapsed = std::chrono::high_resolution_clock::now() - start_time;
            if (elapsed.count() > 15) {
                print_line("Pop Manager Month Tick took " + String::num_scientific(elapsed.count()) + " seconds");
            }
            print_line("Notify");
            jobs_done_cv.notify_all();  // Wake main thread
        }
        lock.unlock();
    }
}


PopManagerThreadPool::PopManagerThreadPool(int num_of_threads, std::function<int()> p_work_adder_function) {
    work_adder_function = std::move(p_work_adder_function);
    for (int i = 0; i < num_of_threads; i++) {
        worker_threads.emplace_back(&PopManagerThreadPool::thread_processor, this);
    }
    month_tick_checker = std::thread(&PopManagerThreadPool::month_tick_check, this);
}

PopManagerThreadPool::~PopManagerThreadPool() {
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

void PopManagerThreadPool::month_tick() {
    {
        std::scoped_lock lock(month_tick_checker_mutex);
        month_tick_flag = true;
    }
    month_tick_checker_cv.notify_one();
}

void PopManagerThreadPool::set_work_function(std::function<void(int)> func) {
    work_function = std::move(func);
}