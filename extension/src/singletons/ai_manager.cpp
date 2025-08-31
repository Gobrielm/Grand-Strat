#include "ai_manager.hpp"
#include "money_controller.hpp"
#include "../classes/prospector_ai.hpp"
#include "../classes/base_pop.hpp"

std::shared_ptr<AiManager> AiManager::singleton_instance = nullptr;

AiManager::AiManager() {
    thread_pool = new ThreadPool<CompanyAi*>(1, [this] () { work_adder_function(); });
}

AiManager::~AiManager() {
    delete thread_pool;
}
    
void AiManager::create() {
    if (singleton_instance == nullptr) {
        singleton_instance = std::make_shared<AiManager>();
    }
}

void AiManager::cleanup() {
    singleton_instance = nullptr;
}

std::shared_ptr<AiManager> AiManager::get_instance() {
    return singleton_instance;
}

void AiManager::work_adder_function() {
    for (const auto& [__, ai]: ais) {
        thread_pool->add_work(ai);
    }
}

int AiManager::create_prospector_ai(int p_country_id, int type) {
    auto ai = memnew(ProspectorAi(p_country_id, --number_of_ais, type));
    {
        std::scoped_lock lock(m);
        ais[ai->get_owner_id()] = ai;
    }
    MoneyController::get_instance()->add_peer(ai->get_owner_id());
    
    return ai->get_owner_id();
}

int AiManager::get_prospector_ai_that_needs_investment(int p_country_id, int type) const {
    std::scoped_lock lock(m);
    for (const auto& [id, ai]: ais) {
        ProspectorAi* prospect_ai = dynamic_cast<ProspectorAi*>(ai);
        if (prospect_ai != nullptr && prospect_ai->get_cargo_type() == type && prospect_ai->needs_investment_from_pops()) {
            return id; // Returns first available
        }
    }
    return 0;
}

void AiManager::employ_pop(int owner_ai_id, int pop_id) {
    {
        std::scoped_lock lock(m);
        ais[owner_ai_id]->employ_pop(pop_id);
    }
}

void AiManager::month_tick() {
    thread_pool->month_tick();
}
