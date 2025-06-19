#include "province_manager.hpp"
#include <godot_cpp/core/class_db.hpp>
#include <chrono>

using namespace godot;

Ref<ProvinceManager> ProvinceManager::singleton_instance = nullptr;

ProvinceManager::ProvinceManager() {
    for (int i = 0; i < 6; i++) {
        worker_threads.push_back(std::thread(&ProvinceManager::thread_processor, this));
    }
}

ProvinceManager::~ProvinceManager() {
    stop = true;
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
    ClassDB::bind_method(D_METHOD("get_province", "province_id"), &ProvinceManager::get_province);

    // Country to province mapping
    ClassDB::bind_method(D_METHOD("add_province_to_country", "province", "country_id"), &ProvinceManager::add_province_to_country);
    ClassDB::bind_method(D_METHOD("get_countries_provinces", "country_id"), &ProvinceManager::get_countries_provinces);

    ClassDB::bind_method(D_METHOD("month_tick"), &ProvinceManager::month_tick);
}

void ProvinceManager::create() {
    if (singleton_instance.is_valid()) return;
    singleton_instance.instantiate();
}

Ref<ProvinceManager> ProvinceManager::get_instance() {
    return singleton_instance;
}

void ProvinceManager::create_new_if_empty(int province_id) {
    std::scoped_lock lock(m);
    if (provinces.find(province_id) == provinces.end()) {
        provinces[province_id] = memnew(Province(province_id));
    }
}

void ProvinceManager::add_tile_to_province(int province_id, Vector2i tile) {
    ERR_FAIL_COND(tiles_to_province_id.count(tile));
    std::scoped_lock lock(m);
    tiles_to_province_id[tile] = province_id;
    provinces[province_id]->add_tile(tile);
}

void ProvinceManager::add_many_tiles_to_province(int province_id, const Array &tiles) {
    for (int i = 0; i < tiles.size(); i++) {
        add_tile_to_province(province_id, tiles[i]);
    }
}

void ProvinceManager::add_population_to_province(Vector2i tile, int pop) {
    int id = get_province_id(tile);
    get_province(id)->add_population(pop);
}

int ProvinceManager::get_province_population(Vector2i tile) {
    int id = get_province_id(tile);
    return get_province(id)->get_population();
}

int ProvinceManager::get_population(int province_id) {
    return get_province(province_id)->get_population();
}

int ProvinceManager::get_population_as_level(int province_id) {
    return get_population(province_id) / 50000;
}

int ProvinceManager::get_total_population() {
    int total = 0;
    for (auto &[_, prov] : provinces) {
        total += prov->get_population();
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
    std::scoped_lock lock(m);
    return tiles_to_province_id.count(tile);
}

int ProvinceManager::get_province_id(Vector2i tile) const {
    std::scoped_lock lock(m);
    auto it = tiles_to_province_id.find(tile);
    return it != tiles_to_province_id.end() ? it->second : -1;
}

Province* ProvinceManager::get_province(int id) const {
    std::scoped_lock lock(m);
    auto it = provinces.find(id);
    return it != provinces.end() ? it->second : nullptr;
}

void ProvinceManager::add_province_to_country(Province* prov, int country_id) {
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
    std::unordered_set<int> s;
    auto it = country_id_to_province_ids.find(country_id);
    if (it != country_id_to_province_ids.end()) {
        for (int id : it->second) {
            s.insert(id);
        }
    }
    return s;
}

void ProvinceManager::month_tick() {
    std::unique_lock<std::mutex> lock(jobs_done_mutex);
    jobs_done_cv.wait(lock, [this](){ // Sleeps and waits for jobs to be done 
        return jobs_remaining == 0; 
    });

    month_tick_helper();
}

void ProvinceManager::month_tick_helper() {
    {
        std::lock_guard<std::mutex> lock(m);
        for (const auto &[__, province]: provinces) {
            provinces_to_process.push_back(province);
        }
        jobs_remaining  = provinces_to_process.size();
    }
    
    condition.notify_all();
}   

void ProvinceManager::thread_processor() {
    while (!stop) {
        Province* to_process = nullptr;

        {
            std::unique_lock<std::mutex> lock(m);
            condition.wait(lock, [this]() {
                return !provinces_to_process.empty() || stop;
            });

            if (stop && provinces_to_process.empty()) {
                return; // Exit thread
            }

            // Get one province to process
            to_process = provinces_to_process.back();
            provinces_to_process.pop_back();

            if (--jobs_remaining == 0) {
                std::lock_guard<std::mutex> lock(jobs_done_mutex);
                jobs_done_cv.notify_one();  // Wake main thread
            }
        }

        to_process->month_tick();
    }
}