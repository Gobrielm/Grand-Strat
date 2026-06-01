#include "province_pop_manager.h"
#include "singletons/pop_manager.hpp"
#include "singletons/terminal_map.hpp"
#include "singletons/province_manager.hpp"
#include "singletons/cargo_info.hpp"
#include "singletons/data_collector.hpp"
#include "utility/debug_trace.h"

#include "classes/province.hpp"
#include "classes/map_objects/town.hpp"
#include "classes/components/employer_component.hpp"

#include <godot_cpp/classes/ref.hpp>

#include <queue>

ProvincePopManager::ProvincePopManager() {
}

ProvincePopManager::~ProvincePopManager() {
    for (auto& [id, __]: pops) {
        fire_pop(id);
    }
}

// void ProvincePopManager::adjust_pop_orders(Town& town) {
//     for (auto& [id, pop]: pops) {
//         pop.adjust_pop_orders(town);
//     }
// }

void ProvincePopManager::pop_tick(Province* province) {
    DebugTrace::get_instance()->log("Pop tick start");
    for (auto& [id, pop]: pops) {
        pop.month_tick();
    }
    DebugTrace::get_instance()->log("Selling");

    sell_to_pops(province);

    // auto time1 = std::chrono::high_resolution_clock::now();
    
    // auto time2 = std::chrono::high_resolution_clock::now();
    // DebugTrace::get_instance()->log("Employment");
    find_employment_for_pops(province);
    // auto time3 = std::chrono::high_resolution_clock::now();

    // String x;
    // std::chrono::duration<double> elapsed1 = time1 - start_time;
    // std::chrono::duration<double> elapsed2 = time2 - time1;
    // std::chrono::duration<double> elapsed3 = time3 - time2;


    // if (elapsed1 > elapsed3) {
    //     x = "Month Tick";
    // } else {
    //     x = "Employment";
    // }

    // std::chrono::duration<double> elapsed = std::chrono::high_resolution_clock::now() - start_time;
    // if (elapsed.count() > 0.5) {
    //     print_line("Pop group month tick took " + String::num_scientific(elapsed1.count()) + " seconds");
    //     print_line("Pop group market tick took " + String::num_scientific(elapsed2.count()) + " seconds");
    //     print_line("Pop group employment tick took " + String::num_scientific(elapsed3.count()) + " seconds");
    //     print_line("Total: " + String::num_scientific(elapsed.count()) + " seconds");
    // }
}

void ProvincePopManager::sell_to_pops(Province* province) {
    DebugTrace::get_instance()->log("Province id: " + std::to_string(province->get_province_id()));
    Town& town = province->town;
    for (auto& [id, pop]: pops) {
        town.mp.sell_to_pop(province, pop);
    }
}

// void ProvincePopManager::create_pop_location_to_towns(std::vector<BasePop*>& pop_group, std::unordered_map<Vector2i, Vector2i, godot_helpers::Vector2iHasher>& location_to_nearest_town) const { 
//     auto province_manager = ProvinceManager::get_instance(); // Get closest town and then use town functions to sell to those pops 
//     { 
//         for (auto& pop: pop_group) { 
//             Vector2i location = pop->get_location(); // Not locking since its not that important for current info
//             if (location_to_nearest_town.count(location)) continue;
//             auto province = province_manager->get_province(location);
//             if (!province -> has_closest_town_tile_to_pop(location)) continue; // No Towns, ie no place to buy from
//             Vector2i town_tile = province->get_closest_town_tile_to_pop(location); 
//             location_to_nearest_town[location] = town_tile;
//         }
//     }
// }
// Vector2i ProvincePopManager::get_town_tile(const BasePop* pop) const {
//     auto province_manager = ProvinceManager::get_instance(); // Get closest town and then use town functions to sell to those pops 
//     Vector2i location = pop->get_location(); // Not locking since its not that important for current info
//     auto province = province_manager->get_province(location);
//     Vector2i town_tile = province->get_closest_town_tile_to_pop(location); 
//     return town_tile;
// }

// void ProvincePopManager::change_pop_unsafe(BasePop * pop) {
//     if (pop->will_degrade()) {
//         pop->degrade();
//     } else if (pop->will_upgrade() && rand() % 5 == 0) {
//         int free_jobs = employment_options[pop->get_type()][get_pop_country_id_unsafe(pop)].size();
//         if (free_jobs > 0) pop->upgrade();
//     }
// }

void ProvincePopManager::find_employment_for_pops(Province* province) {

    Town& town = province->town;
    // Create employment queue
    std::vector<std::priority_queue<std::pair<float, int>>> employement_queue(2);

    for (auto& factory: province->factories) {
        if (factory.is_hiring(PopTypes::rural)) {
            employement_queue[0].push(std::make_pair(factory.get_wage(town), factory.position.get_building_id()));
        }
        if (factory.is_hiring(PopTypes::town)) {
            employement_queue[1].push(std::make_pair(factory.get_wage(town), factory.position.get_building_id()));
        }
    }

    // Employ Pops
    for (auto& [id, pop]: pops) {
        auto pop_type = pop.get_type();
        if (!pop.is_seeking_employment()) continue;
        if (pop_type == PopTypes::peasant) continue; // Do not allow peasants to change jobs
        int pop_type_index = int(pop_type) - 1;
        if (pop_type_index != 0 && pop_type_index != 1) {
            print_line("Pop Type: " + String::num_int64(pop_type_index + 1) + " looking");
            continue;
        }
        
        while (employement_queue[pop_type_index].size() != 0) {
            int building_id = employement_queue[pop_type_index].top().second;
            auto& factory = province->get_factory_unsafe(building_id);
            if (!factory.is_hiring(pop_type)) {
                employement_queue[pop_type_index].pop();
                continue;
            }
            factory.employ_pop(town, pop);
            break;
        }
    }
}

// void ProvincePopManager::find_employment_for_pops(std::vector<BasePop*>& pop_group) {
//     int mutex_lock_num = get_pop_mutex_number(pop_group.front()->get_pop_id());

//     for (auto& pop: pop_group) {
//         PopTypes pop_type = none;
//         {
//             auto lock = lock_pop_read(mutex_lock_num);
//             pop_type = pop->get_type(); // Don't lock since factory will double check if wrong
//             if (!pop->is_seeking_employment()) continue;
//         }
//         employment_finder_helper(pop, pop_type);
//     }
// }

// Currently Pops can move and work anywhere in their country with no penalities
// Also, simple sorting happens by wage, and doesn't consider 


// void ProvincePopManager::employment_finder_helper(BasePop* pop, PopTypes pop_type) {
//     int country_id = get_pop_country_id(pop);
//     // if (pop_type == town && pop->get_wealth() > 50000) { // TODO: Using constant money amount
//     //     bool was_sucessful = employment_for_potential_investor(pop, country_id);
//     //     if (was_sucessful) return;
//     // }

    
//     auto work = get_first_employment_option(pop_type, country_id);
//     if (work.internal_fact == nullptr) return;
//     while (work.internal_fact != nullptr) {
//         if (!work.internal_fact->is_hiring(pop_type)) { 
//             remove_first_employment_option(pop_type, country_id, work.internal_fact); // Erase from set if not hiring assuming the set only has one pop type allowed
//             work = get_first_employment_option(pop_type, country_id); 
//             continue;
//         }
//         bool is_acceptable = false;
//         {
//             auto lock = lock_pop_read(pop->get_pop_id());
//             is_acceptable = pop->is_wage_acceptable(work.wage);
//         }

//         if (is_acceptable) {
//             work.internal_fact->employ_pop(pop, *get_lock(pop->get_pop_id()), pop_type);
//         }
//         break; // Break if found job or not, since wage would only go down
//     }
// }


// bool ProvincePopManager::employment_for_potential_investor(BasePop* pop, int country_id) { // TODO: Test Logic
//     int cargo_type = get_cargo_type_to_build(country_id);
//     if (cargo_type == -1) return false;
//     // print_line(CargoInfo::get_instance()->get_cargo_name(cargo_type));
//     auto terminal_map = TerminalMap::get_instance();
//     Vector2i town_tile;
//     {
//         auto lock = lock_pop_read(pop->get_pop_id());
//         town_tile = pop->get_location();
//     }
//     auto town = get_town_helper(town_tile);
//     ERR_FAIL_COND_V_MSG(!town.has_value(), false, "Town Pop not located on Town");
//     Ref<InvestmentCompany> company = town.value().get_first_invesment_company_looking_for_employees(cargo_type); // Doesn't check for pop_location, migration avoided and searchs country

//     if (company.is_null()) {
//         company = InvestmentCompany::create(country_id, town_tile, cargo_type);
//         TerminalMap::get_instance()->create_isolated_company_in_town(company);
//     }
//     float wealth_transfer = 0;
//     {
//         auto lock = lock_pop_write(pop->get_pop_id());
//         wealth_transfer = pop->transfer_wealth();
//     }
//     company->employ_pop(pop->get_pop_id());
//     float wage = company->get_wage();
//     {
//         auto lock = lock_pop_read(pop->get_pop_id());
//         pop->employ(company->get_terminal_id(), wage);
//     }
//     MoneyController::get_instance()->add_money_to_player(company->get_owner_id(), wealth_transfer);
//     return true;
// }

// int ProvincePopManager::get_cargo_type_to_build(int country_id) const { // TODO: Eventually consider access, volitility, price changing, ect
//     std::unordered_map<int, float> average_prices = ProvinceManager::get_instance()->get_average_country_prices(country_id);
//     if (!average_prices.size()) return -1;
//     auto cmp = [](const std::pair<int, float>& p1,
//                   const std::pair<int, float>& p2) {
//         if (p1.second == p2.second) return p1.first < p2.second;
//         return p1.second > p2.second;
//     };
//     std::set<std::pair<int, float>, decltype(cmp)> s(cmp);
//     for (const auto& [type, price]: average_prices) {
//         const auto employer_component = RecipeInfo::get_instance()->get_employer_component_for_type(type);
//         if (employer_component.has_value()) continue;
//         float exp_profit = get_profit_of_ec(employer_component.value(), average_prices);
//         s.insert({type, exp_profit});
//     }
//     std::vector<int> random_selector = {0, 0, 0, 1, 1, 2, 2, 3, 3, 4};
//     int random = std::min(random_selector[rand() % 10], int(s.size() - 1));
//     const auto& pair_profit = *std::next(s.begin(), random);
//     if (pair_profit.second <= 0) {
//         // print_line(String::num_scientific(pair_profit.second));
//         return -1;
//     } // Shows no profit, maybe not
//     return pair_profit.first;
// }

float ProvincePopManager::get_profit_of_ec(const EmployerComponent ec, const std::unordered_map<int, float>& average_prices) const {
    float profit = 0;
    for (const auto& [type, amount]: ec.recipe.get_inputs()) {
        profit -= average_prices.at(type) * amount;
    }
    for (const auto& [type, amount]: ec.recipe.get_outputs()) {
        profit += average_prices.at(type) * amount;
    }
    for (const auto& [type, amount_of_pops]: ec.get_pops_needed()) {
        for (const auto& [type, amount]: BasePop::get_base_needs(type)) {
            profit -= average_prices.at(type) * amount * amount_of_pops;
        }
    }

    return profit;
}

// void ProvincePopManager::refresh_employment_sorted_by_wage() {
//     {
//         std::scoped_lock lock(employment_mutex);
//         employment_options.clear();
//     }
//     refresh_rural_employment_sorted_by_wage();
//     refresh_town_employment_sorted_by_wage();
// }

// void ProvincePopManager::refresh_rural_employment_sorted_by_wage() {
//     decltype(employment_options) fresh;

//     auto province_manager = ProvinceManager::get_instance();
//     for (int country_id: province_manager->get_country_ids()) {
//         for (const auto& province_id: province_manager->get_country_provinces(country_id)) {
//             auto province = province_manager->get_province(province_id);
//             auto& town = province->town;
//             std::scoped_lock lock(province->m);

//             for (auto& factory: province->factories) {
//                 if (factory.is_hiring(rural)) {
//                     fresh[rural][country_id].insert(std::make_pair(factory.get_wage(town), factory.position.building_id));
//                 }
//             }
//         }
//     }

//     add_local_employment_options(fresh);
// }

// void ProvincePopManager::refresh_town_employment_sorted_by_wage() {
//     decltype(employment_options) fresh;

//     auto province_manager = ProvinceManager::get_instance();
//     for (int country_id: province_manager->get_country_ids()) {
//         for (const auto& province_id: province_manager->get_country_provinces(country_id)) {
//             auto province = province_manager->get_province(province_id);
//             for (const auto& tile: province->get_town_tiles()) {
//                 refresh_town_employment_sorted_by_wage_helper(country_id, tile, fresh);
//             }   
//         }
//     }

//     add_local_employment_options(fresh);
// }

// using employ_type = std::unordered_map<PopTypes, std::unordered_map<int, std::set<FactoryTemplate::FactoryWageWrapper, FactoryTemplate::FactoryWageWrapper::FactoryWageCompare>>>;

// void ProvincePopManager::refresh_town_employment_sorted_by_wage_helper(int country_id, const Vector2i& tile, employ_type& local_employment_options) {
//     auto town = get_town_helper(tile);
//     ERR_FAIL_COND_MSG(!town.has_value(), "Location sent is to a null town");
//     for (const auto& fact: town.value().get_employment_sorted_by_wage(town)) {
//         local_employment_options[PopTypes::town][country_id].insert(FactoryTemplate::FactoryWageWrapper(fact));
//     }
// }

// void ProvincePopManager::add_local_employment_options(employ_type& local_employment_options) {
//     {
//         std::scoped_lock lock(employment_mutex);
//         for (auto& [category, countries] : local_employment_options) {
//             for (auto& [country_id, factories] : countries) {
//                 employment_options[category][country_id].insert(
//                     factories.begin(), factories.end()
//                 );
//             }
//         }
//     }
// }

// FactoryTemplate::FactoryWageWrapper ProvincePopManager::get_first_employment_option(PopTypes pop_type, int country_id) const {
//     std::shared_lock lock(employment_mutex);
//     auto itType = employment_options.find(pop_type);
//     if (itType == employment_options.end()) return FactoryTemplate::FactoryWageWrapper();
//     auto itCountry = itType->second.find(country_id);
//     if (itCountry == itType->second.end() || itCountry->second.empty()) return FactoryTemplate::FactoryWageWrapper();
//     return *(itCountry->second.begin());
// }

// void ProvincePopManager::remove_first_employment_option(PopTypes pop_type, int country_id, const Ref<FactoryTemplate>& double_check) {
//     std::scoped_lock lock(employment_mutex);
//     if (employment_options.count(pop_type) && employment_options.at(pop_type).count(country_id)) {
//         auto first_it = employment_options.at(pop_type).at(country_id).begin();
//         if (first_it == employment_options.at(pop_type).at(country_id).end()) return;
//         if ((first_it->internal_fact).ptr()->get_terminal_id() == double_check.ptr()->get_terminal_id())
//             employment_options[pop_type][country_id].erase(first_it);
//     }
// }

const BasePop* ProvincePopManager::get_pop(int pop_id) const {
    auto it = pops.find(pop_id);
    if (it == pops.end()) {
        ERR_FAIL_V_MSG(nullptr, "Pop accessed at invalid id.");
    }
    return &(it->second);
}

BasePop* ProvincePopManager::get_pop(int pop_id) {
    auto it = pops.find(pop_id);
    if (it == pops.end()) {
        ERR_FAIL_V_MSG(nullptr, "Pop accessed at invalid id.");
    }
    return &(it->second);
}

// Pop Utility
void ProvincePopManager::set_pop_location(int pop_id, const Vector2i& location) {
    pops[pop_id].set_location(location);
}

int ProvincePopManager::create_pop(PopTypes pop_type, Variant culture, Vector2i location, int province_id) {

    auto pop = BasePop(province_id, location, culture, pop_type);
    int id = pop.get_pop_id();
    
    pops.emplace(id, std::move(pop));
    
    return id;
}

void ProvincePopManager::pay_pop(int pop_id, float wage) {
    get_pop(pop_id)->pay_wage(wage);
}

void ProvincePopManager::fire_pop(int pop_id) {
    get_pop(pop_id)->fire();
}

void ProvincePopManager::sell_cargo_to_pop(int pop_id, int type, int amount, float price) {
    get_pop(pop_id)->buy_good(type, amount, price);
}

void ProvincePopManager::give_pop_cargo(int pop_id, int type, int amount) {
    get_pop(pop_id)->add_cargo(type, amount);
}

int ProvincePopManager::get_pop_desired(int pop_id, int type, float price) {
    return get_pop(pop_id)->get_desired(type, price);
}

void ProvincePopManager::pay_pops(int num_to_pay, double for_each) {
    auto it = pops.begin();
    int total = pops.size();
    while (num_to_pay > 0 && it != pops.end()) {
        BasePop& pop = (it)->second;
        int mult = pop.get_type() == PopTypes::town ? 50: 1; // Town pops get *50 bonus to chance
        if ((rand() % int((float(total) / num_to_pay) / mult)) == 0) {
            pop.add_wealth_no_change_to_income(for_each);
            num_to_pay--;
        }
        total--;
        it++;
    }
}

float ProvincePopManager::get_expected_wage(int pop_id) const {
    ERR_FAIL_COND_V_MSG(!pops.count(pop_id), 0.0, "Pop of id: " + String::num(pop_id) + " does not exist");
    auto pop = get_pop(pop_id);
    auto province = ProvinceManager::get_instance()->get_province(pop->get_location());

    std::scoped_lock province_lock(province->m);
    auto town = province->town;
    auto prices = town.mp.get_current_prices();

    return pops.at(pop_id).get_expected_income(prices);
}

int ProvincePopManager::get_number_of_pops() const {
    return pops.size();
}

//Economy stats
std::unordered_map<PopStats, long> ProvincePopManager::get_pop_statistics() const {
    std::unordered_map<PopStats, long> toReturn;

    for (const auto& [id, pop] : pops) {
        if (pop.get_type() == PopTypes::peasant)
            toReturn[PopStats::NumOfPeasants]++;
        if (pop.get_wealth() < 15)
            toReturn[PopStats::NumOfBrokePops]++;
        if (pop.is_starving())
            toReturn[PopStats::NumOfStarvingPops]++;
        if (pop.is_seeking_employment())
            toReturn[PopStats::NumUnemployed]++;
        if (pop.is_unemployed())
            toReturn[PopStats::NumRealUnemployed]++;
    }
    toReturn[PopStats::TotalPopWealth] += get_total_wealth_of_pops();
    return std::move(toReturn);
}

long ProvincePopManager::get_total_wealth_of_pops() const {
    double total = 0;
    for (const auto& [id, pop] : pops) {
        total += pop.get_wealth();
    }
    return std::lround(total);
}

int ProvincePopManager::get_number_of_broke_pops() const {
    int total = 0;
    for (const auto& [pop_id, pop]: pops) {
        if (pop.get_wealth() < 15) {
            total++;
        }
    }
    return total;
}

int ProvincePopManager::get_number_of_starving_pops() const {
    int total = 0;
    for (const auto& [pop_id, pop]: pops) {
        if (pop.is_unemployed()) {
            total++;
        }
    }
    return total;
}

int ProvincePopManager::get_number_of_unemployed_pops() const {
    int total = 0;
    for (const auto& [pop_id, pop]: pops) {
        if (pop.is_unemployed()) {
            total++;
        }
    }
    return total;
}

int ProvincePopManager::get_number_of_real_unemployed_pops() const {
    int total = 0;
    for (const auto& [pop_id, pop]: pops) {
        if (pop.get_income() == 0) {
            total++;
        }
    }
    return total;
}

float ProvincePopManager::get_unemployment_rate() const {
    int total = 0;
    for (const auto& [pop_id, pop]: pops) {
        if (pop.is_unemployed()) {
            total++;
        }
    }
    return total / double(pops.size());
}

float ProvincePopManager::get_real_unemployment_rate() const {
    int total = 0;
    for (const auto& [pop_id, pop]: pops) {
        if (pop.get_income() == 0) {
            total++;
        }
    }
    return total / double(pops.size());
}

std::unordered_map<PopTypes, long> ProvincePopManager::get_pop_type_statistics() const {
    std::unordered_map<PopTypes, long> pop_type_stats;
    for (const auto& [pop_id, pop]: pops) {
        pop_type_stats[pop.get_type()]++;
    }
    return pop_type_stats;
}