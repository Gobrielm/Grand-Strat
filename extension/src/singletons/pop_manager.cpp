#include "pop_manager.hpp"
#include "terminal_map.hpp"
#include "province_manager.hpp"
#include "../classes/province.hpp"
#include "../classes/town.hpp"

std::shared_ptr<PopManager> PopManager::singleton_instance = nullptr;

void PopManager::create() {
    ERR_FAIL_COND_MSG(singleton_instance != nullptr, "PopManger already created.");
    singleton_instance = std::make_shared<PopManager>();
}

PopManager::PopManager() {
    thread_pool = new ThreadPool<BasePop*>(6, [this]() { thread_month_tick_loader(); });
    thread_pool->set_work_function([this](BasePop* pop) {
        month_tick(pop); // use PopManager’s pipeline
    });
    for (int i = 0; i < NUMBER_OF_POP_LOCKS; i++) {
        pop_locks.push_back(new std::shared_mutex);  // constructs a new shared_mutex directly in the vector
    }
}

PopManager::~PopManager() {
    delete thread_pool;
    for (auto& [__, pop]: pops) {
        memdelete (pop);
    }
}

std::shared_ptr<PopManager> PopManager::get_instance() {
    ERR_FAIL_COND_V_MSG(singleton_instance == nullptr, nullptr, "PopManger not created yet.");
    return singleton_instance;
}

void PopManager::thread_month_tick_loader() {
    refresh_rural_employment_sorted_by_wage();
    refresh_town_employment_sorted_by_wage();
    {
        std::scoped_lock sp_pops_lock(m);
        for (const auto [id, pop]: pops) {
            thread_pool->add_work(pop);
        }
    }
    
}

void PopManager::month_tick() {
    thread_pool->month_tick();
}

void PopManager::month_tick(BasePop* pop) { // TODO: Crashes here
    {
        auto lock = lock_pop_write(pop->get_pop_id());
        pop->month_tick();
    }
    // sell_to_pop(pop);
    change_pop(pop);
    find_employment_for_pop(pop);
}

void PopManager::sell_to_pop(BasePop* pop) {
    Ref<TerminalMap> terminal_map = TerminalMap::get_instance();
    auto province_manager = ProvinceManager::get_instance();
    // Get closest town and then use town functions to sell to those pops
    Vector2i location;
    {
        get_lock(pop->get_pop_id());
        location = pop->get_location();
    } 
    auto province = province_manager->get_province(location);
    if (province -> get_town_tiles().size() == 0) return; // No Towns, ie no place to buy from
    Vector2i town_tile = province->get_closest_town_tile_to_pop(location);

    Ref<Town> town = terminal_map->get_town(town_tile);
    if (town.is_null()) {
        print_error("Towns and terminals are desynced at " + pop->get_location());
        return;
    }
    town->sell_to_pop(pop, *get_lock(pop->get_pop_id()));
}

void PopManager::change_pop(BasePop * pop) {
    get_lock(pop->get_pop_id());
    if (pop->will_degrade()) {
        pop->degrade();
    } else if (pop->will_upgrade()) {
        pop->upgrade();
    }
}

void PopManager::find_employment_for_pop(BasePop* pop) { // No locking
    if (pop->get_type() == rural) {
        find_employment_for_rural_pop(pop);
    } else if (pop->get_type() == town) {
        find_employment_for_town_pop(pop);
    }
}

// Currently Pops can move and work anywhere in their country with no penalities
// Also, simple sorting happens by wage, and doesn't consider 

void PopManager::find_employment_for_rural_pop(BasePop* pop) { 
    auto& employment_options = rural_employment_options[get_pop_country_id(pop)];
    employment_finder_helper(pop, employment_options);
}

void PopManager::find_employment_for_town_pop(BasePop* pop) {
    auto& employment_options = town_employment_options[get_pop_country_id(pop)];
    employment_finder_helper(pop, employment_options);
}

void PopManager::employment_finder_helper(BasePop* pop, std::set<godot::Ref<FactoryTemplate>, FactoryTemplate::FactoryWageCompare> &employment_options) {
    auto it = employment_options.begin();
    Ref<FactoryTemplate> work = *it;
    {
        auto lock = lock_pop_read(pop->get_pop_id());
        if (!pop->is_seeking_employment()) return;
    }

    while (it != employment_options.end()) {
        if (!work->is_hiring(pop->get_type())) { 
            it = employment_options.erase(it); // Erase from set if not hiring assuming the set only has one pop type allowed
            continue;
        }
        float wage = work->get_wage();
        bool is_acceptable = false;
        {
            auto lock = lock_pop_read(pop->get_pop_id());
            is_acceptable = pop->is_wage_acceptable(wage);
        }

        if (is_acceptable) {
            work->employ_pop(pop, *get_lock(pop->get_pop_id()));
        }
        break; // Break if found job or not
    }
}

void PopManager::refresh_rural_employment_sorted_by_wage() {
    auto province_manager = ProvinceManager::get_instance();
    {
        std::scoped_lock lock(m);
        rural_employment_options.clear();
    }
    

    for (int country_id: province_manager->get_country_ids()) {
        for (const auto& province_id: province_manager->get_country_provinces(country_id)) {
            auto province = province_manager->get_province(province_id);
            for (const auto& tile: province->get_tiles_vector()) {
                refresh_rural_employment_sorted_by_wage_helper(country_id, tile);
            }   
            
        }
    }
}

void PopManager::refresh_rural_employment_sorted_by_wage_helper(int country_id, const Vector2i& tile) {
    Ref<FactoryTemplate> fact = TerminalMap::get_instance() -> get_terminal_as<FactoryTemplate>(tile);
    if (fact.is_null()) return;

    if (fact->is_hiring(rural)) {
        std::scoped_lock lock(m);
        rural_employment_options[country_id].insert(fact);
    }
}

void PopManager::refresh_town_employment_sorted_by_wage() {
    auto province_manager = ProvinceManager::get_instance();
    {
        std::scoped_lock lock(m);
        town_employment_options.clear();
    }
    
    for (int country_id: province_manager->get_country_ids()) {
        for (const auto& province_id: province_manager->get_country_provinces(country_id)) {
            auto province = province_manager->get_province(province_id);
            for (const auto& tile: province->get_town_tiles()) {
                refresh_town_employment_sorted_by_wage_helper(country_id, tile);
            }   
            
        }
    }
}

void PopManager::refresh_town_employment_sorted_by_wage_helper(int country_id, const Vector2i& tile) {
    Ref<Town> town_ref = TerminalMap::get_instance()->get_town(tile);
    ERR_FAIL_COND_MSG(town_ref.is_null(), "Location sent is to a null town");
    for (const auto& fact: town_ref->get_employment_sorted_by_wage(town)) {
        std::scoped_lock lock(m);
        town_employment_options[country_id].insert(fact);
    }
}


std::shared_mutex* PopManager::get_lock(int pop_id) {
    return (pop_locks.at(pop_id % NUMBER_OF_POP_LOCKS));
}

std::shared_lock<std::shared_mutex> PopManager::lock_pop_read(int pop_id) const {
    return std::shared_lock(*pop_locks[pop_id % NUMBER_OF_POP_LOCKS]);
}

std::unique_lock<std::shared_mutex> PopManager::lock_pop_write(int pop_id) const {
    return std::unique_lock(*pop_locks[pop_id % NUMBER_OF_POP_LOCKS]);
}

BasePop* PopManager::get_pop(int pop_id) const {
    std::shared_lock lock(m);
    ERR_FAIL_COND_V_MSG(!pops.count(pop_id), nullptr, "Pop accessed at invalid id.");
    return pops.at(pop_id);
}

int PopManager::get_pop_country_id(BasePop* pop) const {
    Vector2i location = pop->get_location();
    return ProvinceManager::get_instance()->get_province(location)->get_country_id();
}

// Pop Utility
void PopManager::set_pop_location(int pop_id, const Vector2i& location) {
    auto lock = lock_pop_write(pop_id);
    get_pop(pop_id)->set_location(location);
}

int PopManager::create_pop(Variant culture, const Vector2i& p_location, PopTypes p_pop_type) {
    int home_prov_id = ProvinceManager::get_instance()->get_province_id(p_location);
    auto pop = memnew(BasePop(home_prov_id, p_location, culture, p_pop_type));
    {
        std::unique_lock lock(m);
        pops[pop->get_pop_id()] = pop;
    }
    return pop->get_pop_id();
}

void PopManager::pay_pop(int pop_id, float wage) {
    auto lock = lock_pop_write(pop_id);
    get_pop(pop_id)->pay_wage(wage);
}

void PopManager::fire_pop(int pop_id) {
    auto lock = lock_pop_write(pop_id);
    get_pop(pop_id)->fire();
}

void PopManager::sell_cargo_to_pop(int pop_id, int type, int amount, float price) {
    auto lock = lock_pop_write(pop_id);
    get_pop(pop_id)->buy_good(type, amount, price);
}

void PopManager::give_pop_cargo(int pop_id, int type, int amount) {
    auto lock = lock_pop_write(pop_id);
    get_pop(pop_id)->add_cargo(type, amount);
}

int PopManager::get_pop_desired(int pop_id, int type, float price) {
    auto lock = lock_pop_write(pop_id);
    return get_pop(pop_id)->get_desired(type, price);
}

void PopManager::pay_pops(int num_to_pay, double for_each) {
    std::shared_lock lock(m);
    auto it = pops.begin();
    int total = pops.size();
    while (num_to_pay > 0 && it != pops.end()) {
        if (total % num_to_pay == 0) {
            BasePop* pop = (it)->second;
            auto lock = lock_pop_write(it->first);
            pop->add_wealth(num_to_pay);
            num_to_pay--;
        }
        total--;
        it++;
    }
}

//Economy stats
float PopManager::get_average_cash_of_pops() const {
    double total = 0;
    std::scoped_lock sp_pops_lock(m);
    for (const auto& [pop_id, pop]: pops) {
        auto lock = lock_pop_read(pop_id);
        total += pop->get_wealth();
    }
    return total / pops.size();
}

int PopManager::get_number_of_broke_pops() const {
    int total = 0;
    std::scoped_lock sp_pops_lock(m);
    for (const auto& [pop_id, pop]: pops) {
        auto lock = lock_pop_read(pop_id);
        if (pop->get_wealth() < 15) {
            total++;
        }
    }
    return total;
}

int PopManager::get_number_of_starving_pops() const {
    int total = 0;
    std::scoped_lock sp_pops_lock(m);
    for (const auto& [pop_id, pop]: pops) {
        auto lock = lock_pop_read(pop_id);
        if (pop->is_starving()) {
            total++;
        }
    }
    return total;
}

float PopManager::get_unemployment_rate() const {
    int total = 0;
    std::scoped_lock sp_pops_lock(m);
    for (const auto& [pop_id, pop]: pops) {
        auto lock = lock_pop_read(pop_id);
        if (pop->is_unemployed()) {
            total++;
        }
    }
    return total / double(pops.size());
}

float PopManager::get_real_unemployment_rate() const {
    int total = 0;
    std::scoped_lock sp_pops_lock(m);
    for (const auto& [pop_id, pop]: pops) {
        auto lock = lock_pop_read(pop_id);
        if (pop->get_income() == 0) {
            total++;
        }
    }
    return total / double(pops.size());
}

std::unordered_map<PopTypes, int> PopManager::get_pop_type_statistics() const {
    std::unordered_map<PopTypes, int> pop_type_stats;
    std::scoped_lock sp_pops_lock(m);
    for (const auto& [pop_id, pop]: pops) {
        auto lock = lock_pop_read(pop_id);
        pop_type_stats[pop->get_type()]++;
    }
    return pop_type_stats;
}