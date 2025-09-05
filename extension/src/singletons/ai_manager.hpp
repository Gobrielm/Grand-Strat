#pragma once

#include <unordered_map>
#include <mutex>
#include <memory>
#include "../classes/company_ai.hpp"
#include "../utility/thread_pool.hpp"

class BasePop;

// CURRENTLY NOT USED ANYMORE, WILL PROBS BE USED FOR COUNTRY AIs

class AiManager {

private:
    mutable std::shared_mutex m;
    static std::shared_ptr<AiManager> singleton_instance;

    std::atomic<int> number_of_ais = 0;
    std::unordered_map<int, CompanyAi*> ais;
    ThreadPool<CompanyAi*>* thread_pool;

    void work_adder_function();

public:
    AiManager();
    ~AiManager();
    
    static void create();
    static void cleanup();
    static std::shared_ptr<AiManager> get_instance();

    void employ_pop(int owner_ai_id, int pop_id);

    void month_tick();
};
