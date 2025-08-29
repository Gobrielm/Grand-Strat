#include "pop_manager.hpp"
#include "terminal_map.hpp"
#include "province_manager.hpp"
#include "../classes/province.hpp"
#include "../classes/town.hpp"
#include "../classes/prospector_ai.hpp"

std::shared_ptr<PopManager> PopManager::singleton_instance = nullptr;

void PopManager::create() {
    ERR_FAIL_COND_MSG(singleton_instance != nullptr, "PopManger already created.");
    singleton_instance = std::make_shared<PopManager>();
}

void PopManager::cleanup() {
    singleton_instance.reset();
}

PopManager::PopManager() {
    thread_pool = new PopManagerThreadPool(8, [this]() { return thread_month_tick_loader(); });

    thread_pool->set_work_function([this] (std::vector<BasePop*>& pops) {
        month_tick(pops); // use PopManager’s pipeline
    });
    for (int i = 0; i < NUMBER_OF_POP_LOCKS; i++) {
        pop_locks.push_back(new std::shared_mutex);  // constructs a new shared_mutex directly in the vector
    }
}

PopManager::~PopManager() {
    delete thread_pool;
}

std::shared_ptr<PopManager> PopManager::get_instance() {
    ERR_FAIL_COND_V_MSG(singleton_instance == nullptr, nullptr, "PopManger not created yet.");
    return singleton_instance;
}

int PopManager::thread_month_tick_loader() { // TESTING WITHOUT EMPLOYMENT TO SEE IF PROBLEM
    refresh_employment_sorted_by_wage();
    {
        std::scoped_lock sp_pops_lock(m);
        for (auto& [id, pop]: pops) {
            thread_pool->add_work((&pop), pop.get_pop_id() % NUMBER_OF_POP_LOCKS);
        }
    }
    return pops.size();
}

void PopManager::month_tick() {
    thread_pool->month_tick();
}
// If full month tick takes a while, all pop group ticks take a while
void PopManager::month_tick(std::vector<BasePop*>& pop_group) { // Assumes all pops are part of same mutex block
    auto start_time = std::chrono::high_resolution_clock::now();
    int mutex_lock_num = get_pop_mutex_number(pop_group.front()->get_pop_id());
    {
        auto lock = lock_pop_write(mutex_lock_num);
        for (auto &pop: pop_group) {
            pop->month_tick();
            change_pop_unsafe(pop);
        }
    }
    auto time1 = std::chrono::high_resolution_clock::now();
    
    sell_to_pops(pop_group);
    auto time2 = std::chrono::high_resolution_clock::now();
    find_employment_for_pops(pop_group);
    auto time3 = std::chrono::high_resolution_clock::now();

    String x;
    std::chrono::duration<double> elapsed1 = time1 - start_time;
    std::chrono::duration<double> elapsed2 = time2 - time1;
    std::chrono::duration<double> elapsed3 = time3 - time2;
    if (elapsed1 > elapsed2 && elapsed1 > elapsed3) {
        x = "Month Tick";
    } else if (elapsed2 > elapsed1 && elapsed2 > elapsed3) {
        x = "Sell To Pops";
    } else {
        x = "Employment";
    }

    std::chrono::duration<double> elapsed = std::chrono::high_resolution_clock::now() - start_time;
    if (elapsed.count() > 0.5) {
        print_line("Pop group " + x + " tick took " + String::num_scientific(elapsed.count()) + " seconds");
    }
}


void PopManager::sell_to_pops(std::vector<BasePop*>& pop_group) {
    Ref<TerminalMap> terminal_map = TerminalMap::get_instance();
    std::unordered_map<Vector2i, Vector2i, godot_helpers::Vector2iHasher> location_to_nearest_town;
    create_pop_location_to_towns(pop_group, location_to_nearest_town);
    // Get closest town and then use town functions to sell to those pops
    
    for (auto& pop: pop_group) {
        Ref<Town> town = terminal_map->get_town(location_to_nearest_town[pop->get_location()]);
        if (town.is_null()) {
            continue;
        }
        town->sell_to_pop(pop, *get_lock(pop->get_pop_id()));
    }
}

void PopManager::create_pop_location_to_towns(std::vector<BasePop*>& pop_group, std::unordered_map<Vector2i, Vector2i, godot_helpers::Vector2iHasher>& location_to_nearest_town) const { 
    auto province_manager = ProvinceManager::get_instance(); // Get closest town and then use town functions to sell to those pops 
    { 
        for (auto& pop: pop_group) { 
            Vector2i location = pop->get_location(); // Not locking since its not that important for current info
            if (location_to_nearest_town.count(location)) continue;
            auto province = province_manager->get_province(location);
            if (!province -> has_closest_town_tile_to_pop(location)) continue; // No Towns, ie no place to buy from
            Vector2i town_tile = province->get_closest_town_tile_to_pop(location); 
            location_to_nearest_town[location] = town_tile;
        }
    }
}

void PopManager::change_pop_unsafe(BasePop * pop) {
    if (pop->will_degrade()) {
        pop->degrade();
    } else if (pop->will_upgrade()) {
        pop->upgrade();
    }
}

void PopManager::find_employment_for_pops(std::vector<BasePop*>& pop_group) {
    int mutex_lock_num = get_pop_mutex_number(pop_group.front()->get_pop_id());

    for (auto& pop: pop_group) {
        PopTypes pop_type = none;
        {
            auto lock = lock_pop_read(mutex_lock_num);
            pop_type = pop->get_type(); // Don't lock since factory will double check if wrong
            if (!pop->is_seeking_employment()) continue;
        }
        employment_finder_helper(pop, pop_type);
    }
}

// Currently Pops can move and work anywhere in their country with no penalities
// Also, simple sorting happens by wage, and doesn't consider 


void PopManager::employment_finder_helper(BasePop* pop, PopTypes pop_type) {
    int country_id = get_pop_country_id(pop);
    if (pop_type == town && pop->get_wealth() > 5000) {
        bool was_sucessful = employment_for_potential_investor(pop, country_id);
        if (was_sucessful) return;
    }

    
    auto work = get_first_employment_option(pop_type, country_id);
    if (work.internal_fact == nullptr) return;
    while (work.internal_fact != nullptr) {
        if (!work.internal_fact->is_hiring(pop_type)) { 
            remove_first_employment_option(pop_type, country_id, work.internal_fact); // Erase from set if not hiring assuming the set only has one pop type allowed
            work = get_first_employment_option(pop_type, country_id); 
            continue;
        }
        bool is_acceptable = false;
        {
            auto lock = lock_pop_read(pop->get_pop_id());
            is_acceptable = pop->is_wage_acceptable(work.wage);
        }

        if (is_acceptable) {
            work.internal_fact->employ_pop(pop, *get_lock(pop->get_pop_id()), pop_type);
        }
        break; // Break if found job or not
    }
}

bool PopManager::employment_for_potential_investor(BasePop* pop, int country_id) {
    int type = 10; // TODO: get most lucrious type
    auto ai = ProspectorAi(country_id, -1241251, type);
    ai.employ_pop(pop->get_pop_id());
    //TODO: Store ai somewhere
}

void PopManager::refresh_employment_sorted_by_wage() {
    {
        std::scoped_lock lock(employment_mutex);
        employment_options.clear();
    }
    refresh_rural_employment_sorted_by_wage();
    refresh_town_employment_sorted_by_wage();
}

void PopManager::refresh_rural_employment_sorted_by_wage() {
    decltype(employment_options) fresh;

    auto province_manager = ProvinceManager::get_instance();
    for (int country_id: province_manager->get_country_ids()) {
        for (const auto& province_id: province_manager->get_country_provinces(country_id)) {
            auto province = province_manager->get_province(province_id);
            for (const auto& tile: province->get_terminal_tiles_set()) {
                Ref<FactoryTemplate> fact = TerminalMap::get_instance()->get_terminal_as<FactoryTemplate>(tile);
                if (!fact.is_null() && fact->is_hiring(rural)) {
                    fresh[rural][country_id].insert(fact);
                }
            }
        }
    }

    add_local_employment_options(fresh);
}

void PopManager::refresh_town_employment_sorted_by_wage() {
    decltype(employment_options) fresh;

    auto province_manager = ProvinceManager::get_instance();
    for (int country_id: province_manager->get_country_ids()) {
        for (const auto& province_id: province_manager->get_country_provinces(country_id)) {
            auto province = province_manager->get_province(province_id);
            for (const auto& tile: province->get_town_tiles()) {
                refresh_town_employment_sorted_by_wage_helper(country_id, tile, fresh);
            }   
        }
    }

    add_local_employment_options(fresh);
}

using employ_type = std::unordered_map<PopTypes, std::unordered_map<int, std::set<FactoryTemplate::FactoryWageWrapper, FactoryTemplate::FactoryWageWrapper::FactoryWageCompare>>>;

void PopManager::refresh_town_employment_sorted_by_wage_helper(int country_id, const Vector2i& tile, employ_type& local_employment_options) {
    Ref<Town> town_ref = TerminalMap::get_instance()->get_town(tile);
    ERR_FAIL_COND_MSG(town_ref.is_null(), "Location sent is to a null town");
    for (const auto& fact: town_ref->get_employment_sorted_by_wage(town)) {
        local_employment_options[town][country_id].insert(FactoryTemplate::FactoryWageWrapper(fact));
    }
}

void PopManager::add_local_employment_options(employ_type& local_employment_options) {
    {
        std::scoped_lock lock(employment_mutex);
        for (auto& [category, countries] : local_employment_options) {
            for (auto& [country_id, factories] : countries) {
                employment_options[category][country_id].insert(
                    factories.begin(), factories.end()
                );
            }
        }
    }
}

FactoryTemplate::FactoryWageWrapper PopManager::get_first_employment_option(PopTypes pop_type, int country_id) const {
    std::shared_lock lock(employment_mutex);
    auto itType = employment_options.find(pop_type);
    if (itType == employment_options.end()) return FactoryTemplate::FactoryWageWrapper();
    auto itCountry = itType->second.find(country_id);
    if (itCountry == itType->second.end() || itCountry->second.empty()) return FactoryTemplate::FactoryWageWrapper();
    return *(itCountry->second.begin());
}

void PopManager::remove_first_employment_option(PopTypes pop_type, int country_id, const Ref<FactoryTemplate>& double_check) {
    std::scoped_lock lock(employment_mutex);
    if (employment_options.count(pop_type) && employment_options.at(pop_type).count(country_id)) {
        auto first_it = employment_options.at(pop_type).at(country_id).begin();
        if (first_it == employment_options.at(pop_type).at(country_id).end()) return;
        if ((first_it->internal_fact).ptr()->get_terminal_id() == double_check.ptr()->get_terminal_id())
            employment_options[pop_type][country_id].erase(first_it);
    }
}

int PopManager::get_pop_mutex_number(int pop_id) const {
    return pop_id % NUMBER_OF_POP_LOCKS;
}

std::shared_mutex* PopManager::get_lock(int pop_id) const {
    return (pop_locks.at(get_pop_mutex_number(pop_id)));
}

std::shared_lock<std::shared_mutex> PopManager::lock_pop_read(int pop_id) const {
    return std::shared_lock(*pop_locks[get_pop_mutex_number(pop_id)]);
}

std::unique_lock<std::shared_mutex> PopManager::lock_pop_write(int pop_id) const {
    return std::unique_lock(*pop_locks[get_pop_mutex_number(pop_id)]);
}

BasePop* PopManager::get_pop(int pop_id) {
    std::shared_lock lock(m);
    auto it = pops.find(pop_id);
    if (it == pops.end()) {
        ERR_FAIL_V_MSG(nullptr, "Pop accessed at invalid id.");
    }
    return &(it->second);
}

int PopManager::get_pop_country_id(BasePop* pop) const {
    Vector2i location;
    {
        auto lock = lock_pop_read(pop->get_pop_id());
        location = pop->get_location();
    } 
    return ProvinceManager::get_instance()->get_province(location)->get_country_id();
}

// Pop Utility
void PopManager::set_pop_location(int pop_id, const Vector2i& location) {
    auto lock = lock_pop_write(pop_id);
    get_pop(pop_id)->set_location(location);
}

int PopManager::create_pop(Variant culture, const Vector2i& p_location, PopTypes p_pop_type) {
    int home_prov_id = ProvinceManager::get_instance()->get_province_id(p_location);
    auto pop = BasePop(home_prov_id, p_location, culture, p_pop_type);
    int id = pop.get_pop_id();
    {
        std::unique_lock lock(m);
        pops.emplace(id, std::move(pop));
    }
    return id;
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
    auto lock = lock_pop_read(pop_id);
    return get_pop(pop_id)->get_desired(type, price);
}

void PopManager::pay_pops(int num_to_pay, double for_each) { // Isn't random
    std::shared_lock lock(m);
    auto it = pops.begin();
    int total = pops.size();
    while (num_to_pay > 0 && it != pops.end()) {
        if (total % num_to_pay == 0) {
            BasePop& pop = (it)->second;
            auto lock = lock_pop_write(it->first);
            pop.add_wealth(for_each);
            num_to_pay--;
        }
        total--;
        it++;
    }
}

//Economy stats
std::shared_ptr<std::unordered_map<PopStats, float>> PopManager::get_pop_statistics() const {
    std::shared_ptr<std::unordered_map<PopStats, float>> toReturn = std::make_shared<std::unordered_map<PopStats, float>>();
    std::shared_lock lock(m);
    for (const auto& [id, pop] : pops) {
        auto lock = lock_pop_read(id);
        if (pop.get_type() == peasant)
            (*toReturn)[NumOfPeasants]++;
        if (pop.get_wealth() < 15)
            (*toReturn)[NumOfBrokePops]++;
        if (pop.is_starving())
            (*toReturn)[NumOfStarvingPops]++;
        if (pop.is_seeking_employment())
            (*toReturn)[UnemploymentRate]++;
        if (pop.is_unemployed())
            (*toReturn)[RealUnemploymentRate]++;
        (*toReturn)[AveragePopWealth] += pop.get_wealth();
    }
    (*toReturn)[UnemploymentRate] /= pops.size();
    (*toReturn)[RealUnemploymentRate] /= pops.size();
    (*toReturn)[AveragePopWealth] /= pops.size();
    return toReturn;
}

float PopManager::get_average_cash_of_pops() const {
    std::shared_lock lock(m);
    double total = 0;
    for (const auto& [id, pop] : pops) {
        auto l = lock_pop_read(id);
        total += pop.get_wealth();
    }
    return pops.empty() ? 0.f : float(total / pops.size());
}

int PopManager::get_number_of_broke_pops() const {
    int total = 0;
    std::shared_lock lock(m);
    for (const auto& [pop_id, pop]: pops) {
        auto lock = lock_pop_read(pop_id);
        if (pop.get_wealth() < 15) {
            total++;
        }
    }
    return total;
}

int PopManager::get_number_of_starving_pops() const {
    int total = 0;
    std::shared_lock lock(m);
    for (const auto& [pop_id, pop]: pops) {
        auto lock = lock_pop_read(pop_id);
        if (pop.is_starving()) {
            total++;
        }
    }
    return total;
}

float PopManager::get_unemployment_rate() const {
    int total = 0;
    std::shared_lock lock(m);
    for (const auto& [pop_id, pop]: pops) {
        auto lock = lock_pop_read(pop_id);
        if (pop.is_unemployed()) {
            total++;
        }
    }
    return total / double(pops.size());
}

float PopManager::get_real_unemployment_rate() const {
    int total = 0;
    std::shared_lock lock(m);
    for (const auto& [pop_id, pop]: pops) {
        auto lock = lock_pop_read(pop_id);
        if (pop.get_income() == 0) {
            total++;
        }
    }
    return total / double(pops.size());
}

std::unordered_map<PopTypes, int> PopManager::get_pop_type_statistics() const {
    std::unordered_map<PopTypes, int> pop_type_stats;
    std::shared_lock lock(m);
    for (const auto& [pop_id, pop]: pops) {
        auto lock = lock_pop_read(pop_id);
        pop_type_stats[pop.get_type()]++;
    }
    return pop_type_stats;
}