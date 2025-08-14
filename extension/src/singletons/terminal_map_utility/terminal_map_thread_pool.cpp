#include "terminal_map_thread_pool.hpp"
#include "../terminal_map.hpp"

TerminalMapThreadPool::TerminalMapThreadPool() {
    day_tick_checker = std::thread(&TerminalMapThreadPool::day_tick_check, this);
    month_tick_checker = std::thread(&TerminalMapThreadPool::month_tick_check, this);
    for (int i = 0; i < 4; i++) {
        day_worker_threads.push_back(std::thread(&TerminalMapThreadPool::day_thread_processor, this));
    }
    for (int i = 0; i < 4; i++) {
        month_worker_threads.push_back(std::thread(&TerminalMapThreadPool::month_thread_processor, this));
    }
    terminal_map = TerminalMap::get_instance().ptr();
}

TerminalMapThreadPool::~TerminalMapThreadPool() {
    stop = true;
}

//Public calls

void TerminalMapThreadPool::day_tick() {
    {
        std::unique_lock<std::mutex> lock(day_tick_checker_mutex);
        day_tick_check_requested = true;
    }
    
    day_tick_checker_cv.notify_one();
}

void TerminalMapThreadPool::day_tick_check() {
    while (!stop) {
        {
            std::unique_lock<std::mutex> lock(day_tick_checker_mutex); // Waits for day tick here
            day_tick_checker_cv.wait(lock, [this](){
                return day_tick_check_requested || stop;
            });
            day_tick_check_requested = false;
        }

        terminal_map->pause_time();
        {
            std::unique_lock<std::mutex> lock(day_jobs_done_mutex);
            day_jobs_cv.wait(lock, [this](){ // Waits for last day ticks jobs to be done
                return day_jobs == 0; 
            });
        }
        terminal_map->unpause_time();
        
        day_tick_helper();
    }
}

void TerminalMapThreadPool::month_tick() {
    {
        std::unique_lock<std::mutex> lock(month_tick_checker_mutex);
        month_tick_check_requested = true;
    }
    month_tick_checker_cv.notify_one();
}

void TerminalMapThreadPool::month_tick_check() {
    while (!stop) {
        {
            std::unique_lock<std::mutex> lock(month_tick_checker_mutex); // Waits for day tick here
            month_tick_checker_cv.wait(lock, [this](){
                return month_tick_check_requested || stop;
            });
            month_tick_check_requested = false;
        }


        terminal_map->pause_time();
        {
            std::unique_lock<std::mutex> lock(month_jobs_done_mutex);
            month_jobs_cv.wait(lock, [this](){ // Sleeps and waits for jobs to be done 
                return month_jobs == 0; 
            });
        }
        terminal_map->unpause_time();

        start_time = std::chrono::high_resolution_clock::now();
        month_tick_helper();
    }
}

//Private helpers

void TerminalMapThreadPool::day_tick_helper() {
    for (const auto &terminal: terminal_map->get_terminals_for_day_tick()) {
         day_tick_work.push_front(terminal);
    }
    day_jobs = day_tick_work.size();
    new_day_work.notify_all();
}

void TerminalMapThreadPool::month_tick_helper() {
    for (const auto &terminal: terminal_map->get_terminals_for_month_tick()) {
        month_tick_work.push_back(terminal);
    }
    month_jobs  = month_tick_work.size();
    new_month_work.notify_all();
}

void TerminalMapThreadPool::day_thread_processor() {
    while (!stop) {
        Ref<Terminal> to_process = Ref<Terminal>(nullptr);
        {
            std::unique_lock<std::mutex> lock(m);
            new_day_work.wait(lock, [this]() {
                return !day_tick_work.empty() || stop;
            });

            if (stop && day_tick_work.empty()) {
                return; // Exit thread
            }

            to_process = day_tick_work.back();
            day_tick_work.pop_back();

        }
        to_process->call("day_tick");

        if (--day_jobs == 0) {
            day_jobs_cv.notify_one();  // Wake main thread
        }
    }
}

void TerminalMapThreadPool::month_thread_processor() {
    while (!stop) {
        Ref<Terminal> to_process = Ref<Terminal>(nullptr);
        {
            std::unique_lock<std::mutex> lock(m);
            new_month_work.wait(lock, [this]() {
                return !month_tick_work.empty() || stop;
            });

            if (stop && month_tick_work.empty()) {
                return; // Exit thread
            }

            to_process = month_tick_work.back();
            month_tick_work.pop_back();
        }
        to_process->call("month_tick");

        if (--month_jobs == 0) {
            std::chrono::duration<double> elapsed = std::chrono::high_resolution_clock::now() - start_time;
            print_line("Terminal Map Month Tick Took " + String::num_scientific(elapsed.count()) + " seconds");
            month_jobs_cv.notify_one();  // Wake main thread
        }
    }
}



