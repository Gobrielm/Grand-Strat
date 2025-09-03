#include "province_manager.hpp"
#include "terminal_map.hpp"
#include "../classes/town.hpp"
#include <godot_cpp/core/class_db.hpp>
#include <chrono>

using namespace godot;

Ref<ProvinceManager> ProvinceManager::singleton_instance = nullptr;

ProvinceManager::ProvinceManager() {
}

ProvinceManager::~ProvinceManager() {
}

void ProvinceManager::_bind_methods() {
    ClassDB::bind_static_method("ProvinceManager", D_METHOD("create"), &ProvinceManager::create);
    ClassDB::bind_static_method("ProvinceManager", D_METHOD("get_instance"), &ProvinceManager::get_instance);

    // Province creation
    ClassDB::bind_method(D_METHOD("create_new_if_empty", "province_id"), &ProvinceManager::create_new_if_empty);
    ClassDB::bind_method(D_METHOD("add_tile_to_province", "province_id", "tile"), &ProvinceManager::add_tile_to_province);
    ClassDB::bind_method(D_METHOD("add_many_tiles_to_province", "province_id", "tiles"), &ProvinceManager::add_many_tiles_to_province);

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
}

void ProvinceManager::create() {
    if (singleton_instance.is_valid()) return;
    singleton_instance.instantiate();
}

Ref<ProvinceManager> ProvinceManager::get_instance() {
    return singleton_instance;
}

void ProvinceManager::create_new_if_empty(int province_id) {
    std::unique_lock lock(province_mutex);
    if (provinces.find(province_id) == provinces.end()) {
        provinces[province_id] = memnew(Province(province_id));
    }
}

void ProvinceManager::add_tile_to_province(int province_id, Vector2i tile) {
    ERR_FAIL_COND(tiles_to_province_id.count(tile));
    std::unique_lock lock(province_mutex);
    tiles_to_province_id[tile] = province_id;
    provinces[province_id]->add_tile(tile);
}

void ProvinceManager::add_many_tiles_to_province(int province_id, const Array &tiles) {
    for (int i = 0; i < tiles.size(); i++) {
        add_tile_to_province(province_id, tiles[i]);
    }
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
    for (auto &[_, prov] : provinces) {
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

void ProvinceManager::create_pops_range(MapType::iterator start, MapType::iterator end) {
    for (auto it = start; it != end; it++) {
        (it -> second) -> create_pops();
    }
}

Array ProvinceManager::get_provinces() const {
    Array arr;
    for (auto &[_, prov] : provinces) {
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
    auto it = provinces.find(id);
    if (it == provinces.end()) return nullptr;
    return it->second;
}

Province* ProvinceManager::get_province(int id) const {
    std::shared_lock lock(province_mutex);
    auto it = provinces.find(id);
    if (it == provinces.end()) return nullptr;
    return it->second;
}

Province* ProvinceManager::get_province(const Vector2i& tile) const {
    std::shared_lock lock(province_mutex);
    int id = get_province_id_unsafe(tile);
    auto it = provinces.find(id);
    if (it == provinces.end()) return nullptr;
    return it->second;
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
std::unordered_map<int, float> ProvinceManager::get_average_country_prices(int country_id) const {
    std::unordered_map<int, float> average_prices;
    auto terminal_map = TerminalMap::get_instance();
    int town_count = 0;
    for (const int& province_id: get_country_provinces(country_id)) {
        Province* province = get_province(province_id);
        for (const auto& tile: province->get_town_tiles()) {
            auto town = terminal_map->get_town(tile);
            if (town.is_null()) continue;
            town_count++;
            for (const auto& [type, price]: town->get_local_prices_map()) {
                average_prices[type];
                if (town->get_demand(type) != 0) average_prices[type] += price; // If Demand is 0, then don' consider price
            }
        }
    }
    if (town_count == 0) return average_prices;
    for (auto& [__, price]: average_prices) {
        price /= town_count;
    }
    return average_prices;
}