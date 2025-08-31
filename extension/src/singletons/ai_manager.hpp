#pragma once

#include <unordered_map>
#include <mutex>
#include <memory>
#include "../classes/company_ai.hpp"
#include "../utility/thread_pool.hpp"

class BasePop;

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

    /// @return Returns with the owner id of the ai
    int create_prospector_ai(int p_country_id, int type);
    /// @return Returns with the owner id of the ai that needs investment, 0 if no ai exists
    int get_prospector_ai_that_needs_investment(int p_country_id, int type) const;
    void employ_pop(int owner_ai_id, int pop_id);

    void month_tick();
};
