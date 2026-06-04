#include "province_manager.hpp"
#include "terminal_map.hpp"
#include "pop_manager.hpp"
#include "utility/blocking_thread_pool.h"
#include "utility/debug_trace.h"
#include "classes/map_objects/subsistence_farm.hpp"
#include "classes/map_objects/town.hpp"
#include "classes/map_objects/station.hpp"
#include "cargo_info.hpp"
#include "classes/godot_wrappers/pdh.h"
#include "classes/godot_wrappers/pdp.hpp"
#include <godot_cpp/core/class_db.hpp>
#include <chrono>
#include <thread>

using namespace godot;

Ref<ProvinceManager> ProvinceManager::singleton_instance = nullptr;

ProvinceManager::ProvinceManager() {
    std::function f = [this] () {
        return get_provinces_vector();
    };
    thread_pool = new BlockingThreadPool(4, f);
}

ProvinceManager::~ProvinceManager() {
    if (thread_pool != nullptr) {
        delete thread_pool;
    }
    
    for (auto province: provinces) {
        delete province;
    }
}

void ProvinceManager::_bind_methods() {
    ClassDB::bind_static_method("ProvinceManager", D_METHOD("create"), &ProvinceManager::create);
    ClassDB::bind_static_method("ProvinceManager", D_METHOD("get_instance"), &ProvinceManager::get_instance);

    // Province creation
    ClassDB::bind_method(D_METHOD("create_new_if_empty", "province_id"), &ProvinceManager::create_new_if_empty);
    ClassDB::bind_method(D_METHOD("add_tile_to_province", "province_id", "tile"), &ProvinceManager::add_tile_to_province);
    ClassDB::bind_method(D_METHOD("add_many_tiles_to_province", "province_id", "tiles"), &ProvinceManager::add_many_tiles_to_province);
    ClassDB::bind_method(D_METHOD("finish_province_creation", "province_id"), &ProvinceManager::finish_province_creation);

    // Population handling
    ClassDB::bind_method(D_METHOD("add_population_to_province", "tile", "pop"), &ProvinceManager::add_population_to_province);
    ClassDB::bind_method(D_METHOD("get_province_population", "tile"), &ProvinceManager::get_province_population);
    ClassDB::bind_method(D_METHOD("get_population", "province_id"), &ProvinceManager::get_population);
    ClassDB::bind_method(D_METHOD("get_population_as_level", "province_id"), &ProvinceManager::get_population_as_level);
    ClassDB::bind_method(D_METHOD("get_total_population"), &ProvinceManager::get_total_population);
    ClassDB::bind_method(D_METHOD("create_pops"), &ProvinceManager::create_pops);

    // Province queries
    ClassDB::bind_method(D_METHOD("get_provinces"), &ProvinceManager::get_provinces);
    ClassDB::bind_method(D_METHOD("is_tile_a_province", "tile"), &ProvinceManager::is_tile_a_province);
    ClassDB::bind_method(D_METHOD("get_province_id", "tile"), &ProvinceManager::get_province_id);
    ClassDB::bind_method(D_METHOD("get_province", "province_id"), &ProvinceManager::get_province_godot);

    // Country to province mapping
    ClassDB::bind_method(D_METHOD("add_province_to_country", "province", "country_id"), &ProvinceManager::add_province_to_country);
    ClassDB::bind_method(D_METHOD("get_countries_provinces", "country_id"), &ProvinceManager::get_countries_provinces);

    ClassDB::bind_method(D_METHOD("is_factory", "tile"), &ProvinceManager::is_factory);
    ClassDB::bind_method(D_METHOD("is_town", "tile"), &ProvinceManager::is_town);
    ClassDB::bind_method(D_METHOD("is_station", "tile"), &ProvinceManager::is_station);

    ClassDB::bind_method(D_METHOD("get_town_factories", "town_tile"), &ProvinceManager::get_town_factories);
    ClassDB::bind_method(D_METHOD("get_town_pdps", "town_tile"), &ProvinceManager::get_town_pdps);
    ClassDB::bind_method(D_METHOD("get_town_phps", "town_tile"), &ProvinceManager::get_town_pdhs);
    ClassDB::bind_method(D_METHOD("get_factory_info", "coords"), &ProvinceManager::get_factory_info);
    ClassDB::bind_method(D_METHOD("get_cash_of_factory", "coords"), &ProvinceManager::get_cash_of_factory);
}

void ProvinceManager::create() {
    if (singleton_instance.is_valid()) return;
    singleton_instance.instantiate();
}

Ref<ProvinceManager> ProvinceManager::get_instance() {
    if (singleton_instance.is_null()) {
        ERR_FAIL_V_EDMSG(nullptr, "Province Manager is null.");
    }
    return singleton_instance;
}

Province* ProvinceManager::get_province_private(int province_id) {
    return provinces[province_id_to_vector_position[province_id]];
}

void ProvinceManager::create_new_if_empty(int province_id) {
    std::unique_lock lock(province_mutex);
    if (!province_id_to_vector_position.count(province_id)) {
        province_id_to_vector_position[province_id] = provinces.size();
        provinces.push_back(memnew(Province(province_id)));
    }
}

void ProvinceManager::add_tile_to_province(int province_id, Vector2i tile) {
    ERR_FAIL_COND(tiles_to_province_id.count(tile));
    std::unique_lock lock(province_mutex);
    tiles_to_province_id[tile] = province_id;
    provinces[province_id_to_vector_position[province_id]]->add_tile(tile);
}

void ProvinceManager::add_many_tiles_to_province(int province_id, const Array &tiles) {
    for (int i = 0; i < tiles.size(); i++) {
        add_tile_to_province(province_id, tiles[i]);
    }
}

void ProvinceManager::finish_province_creation(int province_id) {
    get_province_private(province_id)->create_town();
    get_province_private(province_id)->init_province();
}

void ProvinceManager::add_population_to_province(Vector2i tile, int pop) {
    get_province(tile)->add_population(pop);
}

int ProvinceManager::get_province_population(Vector2i tile) {
    return get_province(tile)->get_population();
}

int ProvinceManager::get_population(int province_id) {
    return get_province(province_id)->get_population();
}

int ProvinceManager::get_population_as_level(int province_id) {
    return get_population(province_id) / 50000;
}

int ProvinceManager::get_total_population() const {
    int total = 0;
    for (auto &prov : provinces) {
        total += prov->get_population();
    }
    return total;
}

int ProvinceManager::get_number_of_pops_in_country(int country_id) const {
    int total = 0;
    for (const int& province_id: get_country_provinces(country_id)) {
        Province* province = get_province(province_id);
        total += province->get_number_of_pops();
    }
    return total;
}

void ProvinceManager::create_pops() {
    auto start_time = std::chrono::high_resolution_clock::now();
    std::vector<std::thread> pop_threads;
    auto start = provinces.begin();

    const int NUMBER_OF_THREADS = 4;
    const int chunk_size = provinces.size() / NUMBER_OF_THREADS;

    for (int i = 0; i < NUMBER_OF_THREADS; i++) {
        auto end = start;
        std::advance(end, chunk_size);
        if (i == (NUMBER_OF_THREADS - 1)) {
            end = provinces.end();
        }
        
        std::thread thrd(&ProvinceManager::create_pops_range, this, start, end);
        pop_threads.push_back(std::move(thrd));
        start = end;
    }
    //Wait for threads to finish
    for (auto &thread: pop_threads) {
        thread.join();
    }

    auto end_time = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end_time - start_time;
    print_line("Pop creation took " + String::num_scientific(elapsed.count()) + " seconds");
}

void ProvinceManager::create_pops_range(std::vector<Province*>::iterator start, std::vector<Province*>::iterator end) {
    for (auto it = start; it != end; it++) {
        (*it) -> create_pops();
    }
}

std::vector<int> ProvinceManager::get_provinces_vector() {
    std::vector<int> v;
    for (auto [id, _] : province_id_to_vector_position) {
        v.push_back(id);
    }
    return v;
}

Array ProvinceManager::get_provinces() const {
    Array arr;
    for (auto &prov : provinces) {
        arr.push_back(prov);
    }
    return arr;
}

bool ProvinceManager::is_tile_a_province(Vector2i tile) const {
    std::shared_lock lock(province_mutex);
    return tiles_to_province_id.count(tile);
}

int ProvinceManager::get_province_id(Vector2i tile) const {
    std::shared_lock lock(province_mutex);
    auto it = tiles_to_province_id.find(tile);

    ERR_FAIL_COND_V_MSG(it == tiles_to_province_id.end(), -1, "Tried to find province with tile: " + tile + " which does not exist");
    return it->second;
}

int ProvinceManager::get_province_id_unsafe(Vector2i tile) const {
    std::shared_lock lock(province_mutex);
    auto it = tiles_to_province_id.find(tile);

    if (it == tiles_to_province_id.end()) return -1;
    return it->second;
}

Province* ProvinceManager::get_province_godot(int id) const {
    std::shared_lock lock(province_mutex);
    auto it = province_id_to_vector_position.find(id);
    if (it == province_id_to_vector_position.end()) return nullptr;
    return provinces[it->second];
}

Province* ProvinceManager::get_province(int id) const {
    std::shared_lock lock(province_mutex);
    auto it = province_id_to_vector_position.find(id);
    if (it == province_id_to_vector_position.end()) return nullptr;
    return provinces[it->second];
}

Province* ProvinceManager::get_province(const Vector2i& tile) const {
    std::shared_lock lock(province_mutex);
    int id = get_province_id_unsafe(tile);
    auto it = province_id_to_vector_position.find(id);
    if (it == province_id_to_vector_position.end()) return nullptr;
    return provinces[it->second];
}

bool ProvinceManager::is_tile_available(Vector2i coords) {
    bool traversable = TerminalMap::get_instance()->is_tile_traversable(coords, true);
    auto province = get_province(coords);
    if (province == nullptr) return false;
    bool taken = province->is_building_at_pos(coords);

    return traversable && !taken;
}


void ProvinceManager::add_province_to_country(Province* prov, int country_id) {
    std::unique_lock lock(province_mutex);
    int old_id = prov->get_country_id();
    if (old_id != -1) {
        country_id_to_province_ids[old_id].erase(prov->get_province_id());
    }
    prov->set_country_id(country_id);
    country_id_to_province_ids[country_id].insert(prov->get_province_id());
}

Dictionary ProvinceManager::get_countries_provinces(int country_id) const {
    Dictionary dict;
    auto it = country_id_to_province_ids.find(country_id);
    if (it != country_id_to_province_ids.end()) {
        for (int id : it->second) {
            dict[id] = true;
        }
    }
    return dict;
}

std::unordered_set<int> ProvinceManager::get_country_provinces(int country_id) const {
    auto it = country_id_to_province_ids.find(country_id);
    if (it != country_id_to_province_ids.end()) {
        return it->second;
    }
    return std::unordered_set<int>();
}

std::unordered_set<int> ProvinceManager::get_country_ids() const {
    std::unordered_set<int> s;
    for (const auto& [id, __]: country_id_to_province_ids) {
        s.insert(id);
    }
    return s;
}
// std::unordered_map<int, float> ProvinceManager::get_average_country_prices(int country_id) const {
//     std::unordered_map<int, float> average_prices;
//     auto terminal_map = TerminalMap::get_instance();
//     int town_count = 0;
//     for (const int& province_id: get_country_provinces(country_id)) {
//         Province* province = get_province(province_id);
//         for (const auto& tile: province->get_town_tiles()) {
//             auto town = terminal_map->get_town(tile);
//             if (town.is_null()) continue;
//             town_count++;
//             for (const auto& [type, price]: town->get_local_prices_map()) {
//                 average_prices[type];
//                 if (town->get_demand(type) != 0) average_prices[type] += price; // If Demand is 0, then don' consider price
//             }
//         }
//     }
//     if (town_count == 0) return average_prices;
//     for (auto& [__, price]: average_prices) {
//         price /= town_count;
//     }
//     return average_prices;
// }

bool ProvinceManager::is_factory(Vector2i tile) {
    auto province = get_province(tile);
    if (province == nullptr) return false;
    return BuildingType::FACTORY == province->get_visible_building_type(tile);
}

bool ProvinceManager::is_town(Vector2i tile) {
    auto province = get_province(tile);
    if (province == nullptr) return false;
    return province->get_town_tile() == tile;
}

bool ProvinceManager::is_station(Vector2i tile) {
    auto province = get_province(tile);
    if (province == nullptr) return false;
    return BuildingType::STATION == province->get_visible_building_type(tile);
}

Array ProvinceManager::get_town_factories(Vector2i town_tile) {
    auto province = get_province(town_tile);
    if (province == nullptr) return Array();

    std::scoped_lock lock(province->m);

    auto ids = province->town.get_factory_ids();

    Array a;
    for (auto id: ids) {
        std::scoped_lock lock(province->m);
        Factory& factory = province->get_factory_unsafe(id);
        
        Dictionary d;

        d.set("level", factory.employer.get_level_without_employment());
        d.set("cash", factory.capital.get_cash());
        d.set("recipe", factory.get_recipe().get_recipe_as_string());

        a.push_back(d);
    }

    return a;
}

Dictionary ProvinceManager::get_town_pdps(Vector2i town_tile) {
    auto province = get_province(town_tile);
    if (province == nullptr) return Dictionary();

    std::scoped_lock lock(province->m);

    Town& town = province->town;

    Dictionary d;
    auto price_map = town.mp.get_current_prices();
    for (auto [type, price]: price_map) {
        Ref<PDP> pdp = memnew(PDP(price, town.mp.get_current_supply(type), town.mp.get_current_demand(type), town.mp.get_market_info_plot_godot(type)));
        d[type] = pdp;
    }

    return d;
}

Dictionary ProvinceManager::get_town_pdhs(Vector2i town_tile) {
    auto province = get_province(town_tile);
    if (province == nullptr) return Dictionary();

    std::scoped_lock lock(province->m);

    Town& town = province->town;

    Dictionary d;
    auto price_map = town.mp.get_current_prices();
    for (auto [type, price]: price_map) {
        Ref<PDH> pdp = memnew(PDH(price, town.mp.get_current_supply(type), town.mp.get_current_demand(type), town.mp.get_market_sale_history(type)));
        d[type] = pdp;
    }

    return d;
}

Dictionary ProvinceManager::get_factory_info(const Vector2i coords) {
    auto province = get_province(coords);
    if (province == nullptr) return Dictionary();

    std::scoped_lock lock(province->m);
    PositionComponent pos = province->get_visible_position_component_unsafe(coords);
    Dictionary outer_dict;
    if (pos.get_type() == BuildingType::FACTORY) {
        auto& factory = province->get_factory_unsafe(pos.get_building_id());

        Dictionary cargo_dict;
        for (const auto [type, amount]: factory.storage.get_storage()) {
            Dictionary d;
            d["amount"] = amount;
            d["supply"] = factory.get_current_price(type);
            d["demand"] = factory.get_demand(type);
            cargo_dict[type] = d;
        }

        outer_dict["cargo"] = cargo_dict;
        outer_dict["level"] = factory.employer.get_level_without_employment();
        outer_dict["cash"] = factory.capital.get_cash();
        outer_dict["employment_rate"] = factory.employer.get_employment_rate();
    }

    return outer_dict;
}

int16_t ProvinceManager::get_cash_of_factory(const Vector2i coords) {
    auto province = get_province(coords);
    if (province == nullptr) return 0;

    std::scoped_lock lock(province->m);
    PositionComponent pos = province->get_visible_position_component_unsafe(coords);
    if (pos.get_type() == BuildingType::FACTORY) {
        return round(province->get_factory_unsafe(pos.get_building_id()).capital.get_cash());
    }

    return 0;
}

float ProvinceManager::get_average_factory_level() const {
    double ave = 0;
    int count = 0;

    for (auto province: provinces) {
        std::scoped_lock(province->m);
        for (const auto& factory: province->factories) {
            ave += factory.employer.get_level_without_employment();
            count++;
        }

    }
    return ave / count;
}

float ProvinceManager::get_average_cash_of_sub_farms() const {
    double ave = 0;
    int count = 0;

    for (auto province: provinces) {
        std::scoped_lock(province->m);
        for (const auto& farm: province->sub_farms) {
            ave += farm.capital.get_cash();
            count++;
        }

    }
    return ave / count;
}

float ProvinceManager::get_average_cash_of_factory() const {
    double ave = 0;
    int count = 0;

    for (auto province: provinces) {
        std::scoped_lock(province->m);
        for (const auto& factory: province->factories) {
            ave += factory.capital.get_cash();
            count++;
        }

    }
    return ave / count;
}

float ProvinceManager::get_average_cash_of_station() const {
    double ave = 0;
    int count = 0;

    for (auto province: provinces) {
        std::scoped_lock(province->m);
        for (const auto& station: province->stations) {
            ave += station.capital.get_cash();
            count++;
        }

    }
    return ave / count;
}


unsigned long ProvinceManager::get_grain_demand() const {
    unsigned long total_demand = 0;

    for (auto province: provinces) {
        std::scoped_lock lock(province->m);
        auto& town = province->town;
        int num = (town.mp.get_current_demand(CargoInfo::get_instance()->get_cargo_type("grain")));
        total_demand += num;
    }
    
    return round(total_demand);
}


unsigned long ProvinceManager::get_grain_supply() const {
    double total_demand = 0;

    for (auto province: provinces) {
        std::scoped_lock lock(province->m);
        auto& town = province->town;
        int num = (town.mp.get_current_supply(CargoInfo::get_instance()->get_cargo_type("grain")));
        total_demand += num;
    }
    return round(total_demand);
}

float ProvinceManager::get_average_price(int type) const {
    double total_demand = 0;
    double weighted_average = 0;

    for (auto province: provinces) {
        std::scoped_lock lock(province->m);
        auto& town = province->town;
        
        auto& sale_history = town.mp.get_market_sale_history_ref(type);
        // auto amt = town.mp.get_current_supply(type);
        // auto price = town.mp.get_price(type);

        for (int i = 0; i < sale_history.size(); i++) {
            float price = i / 2.0;
            int amount = sale_history[i];

            total_demand += amount;
            weighted_average += amount * price;
        }
        // total_demand += amt;
        // weighted_average += amt * price;
    }
    if (total_demand == 0) return CargoInfo::get_instance()->get_base_prices()[CargoInfo::get_instance()->get_cargo_type("grain")];
    return weighted_average / total_demand;
}

void ProvinceManager::pay_pops(int num_to_pay, double for_each) {
    while (num_to_pay > 0) {
        int i = rand() % int(provinces.size());
        auto province = provinces[i];
        province->ppm.pay_pops(std::min(10, num_to_pay), for_each);
        num_to_pay -= 10;
    }
}

void ProvinceManager::test_check() {
    long tot = 0;
    long peasant_tot = 0;
    for (auto province: provinces) {
        tot += province->get_demand_for_needed_goods()[CargoInfo::get_instance()->get_cargo_type("grain")];
        peasant_tot += province->get_theoretical_supply_of_grain_from_peasants();
    }
    print_line("Total Grain Needed Exp: " + String::num_int64(tot));
    print_line("Total Grain Produced Peasants Exp: " + String::num_int64(peasant_tot));
}

void ProvinceManager::simulation_tick() {
    auto dt = DebugTrace::get_instance();

    auto popMan = PopManager::get_instance();
    for (auto province: provinces) {
        dt->log("Before Lock");
        std::scoped_lock lock(province->m);
        dt->log("After Lock");
        Town& town = province->town;

        for (auto& factory: province->factories) {
            dt->log("Factory Month Tick");
            factory.month_tick();

            dt->log("Firing");
            // A little sketchy since popMan locks, when ownership transfered to provinceMan, itll be better
            for (const auto pop_id: factory.employer.pops_to_fire) {
                province->ppm.fire_pop(pop_id);
            }
            factory.employer.pops_to_fire.clear();

            dt->log("pay pops");
            for (const auto& [pop_id, _]: factory.employer.get_employee_ids()) {
                float wage = factory.get_wage(town);
                province->ppm.pay_pop(pop_id, wage);
                factory.capital.remove_cash(wage);
            }
        }

        dt->log("Farms");
        for (auto& farm: province->sub_farms) {
            farm.month_tick();

            float wage = farm.get_wage();
            for (const auto& [pop_id, _]: farm.employer.get_employee_ids()) {
                wage = std::min(wage, farm.capital.get_cash());
                province->ppm.pay_pop(pop_id, wage);
                farm.capital.remove_cash(wage);
            }
        }


    }
    dt->log("Sim Done");
}

void ProvinceManager::order_tick() {
    for (auto province: provinces) {
        std::scoped_lock lock(province->m);
        
        Town& town = province->town;
        for (auto& factory: province->factories) {
            factory.adjust_trade_orders(town);
        }
        for (auto& farm: province->sub_farms) {
            farm.adjust_trade_orders(town);
        }
    }
}

void ProvinceManager::bookkeeping_tick() {
    std::function<void(int)> f = [this](int province_id) { 
        auto province = provinces[province_id];
        std::scoped_lock lock(province->m);
        auto& town = province->town;
        town.mp.bookkeeping_tick();
    };

    thread_pool->set_work_function(f);
    auto time_taken = thread_pool->month_tick();
    if (time_taken > 10) {
        print_line("PvMan bookkeeping_tick took: " + String::num(time_taken, 1) + " seconds");
    }

    // for (auto province: provinces) {
    //     std::scoped_lock lock(province->m);
    //     auto& town = province->town;
    //     town.mp.update_last_month_plot();
    // }
}

void ProvinceManager::trading_tick() {
    for (auto prov_id: get_provinces_vector()) {
        auto province = get_province(prov_id);
        
        std::scoped_lock lock(province->m);
        auto& town = province->town;
        town.mp.market_tick(province);
    }
}

