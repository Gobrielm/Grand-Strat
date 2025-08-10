#pragma once

#include <unordered_map>
#include <mutex>
#include <shared_mutex>
#include <thread>
#include <condition_variable>
#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/variant/dictionary.hpp>
#include <godot_cpp/variant/array.hpp>
#include "../classes/province.hpp"

using namespace godot;

class ProvinceManager : public RefCounted {
    GDCLASS(ProvinceManager, RefCounted);


    mutable std::mutex m;
    mutable std::shared_mutex province_mutex;
    static Ref<ProvinceManager> singleton_instance;
    std::unordered_map<int, Province*> provinces;
    std::unordered_map<Vector2i, int, godot_helpers::Vector2iHasher> tiles_to_province_id;
    std::unordered_map<int, std::unordered_set<int>> country_id_to_province_ids;


    std::thread month_tick_checker; //used to check if next day is ready without blocking
    std::mutex month_tick_checker_mutex;
    std::condition_variable month_tick_checker_cv;
    bool month_tick_check_requested = false;

    void month_tick_check();

    void month_tick_helper();
    std::vector<std::thread> worker_threads;
    std::vector<Province*> provinces_to_process;
    std::condition_variable condition;
    std::atomic<bool> stop = false;

    std::atomic<int> jobs_remaining = 0;
    std::condition_variable jobs_done_cv;
    std::mutex jobs_done_mutex;

protected:
    static void _bind_methods();

public:
    ProvinceManager();
    ~ProvinceManager();
    static void create();
    static Ref<ProvinceManager> get_instance();

    // Province creation
    void create_new_if_empty(int province_id);
    void add_tile_to_province(int province_id, Vector2i tile);
    void add_many_tiles_to_province(int province_id, const Array& tiles);

    // Population handling
    void add_population_to_province(Vector2i tile, int pop);
    int get_province_population(Vector2i tile);
    int get_population(int province_id);
    int get_population_as_level(int province_id);
    int get_total_population();
    void create_pops();
    using MapType = std::unordered_map<int, Province*>;
    void create_pops_range(MapType::iterator start, MapType::iterator end);

    // Province queries
    Array get_provinces() const;
    bool is_tile_a_province(Vector2i tile) const;
    int get_province_id(Vector2i tile) const;
    Province* get_province(int province_id) const;

    //Economy stats
    float get_average_cash_of_pops() const;
    int get_number_of_broke_pops() const;
    int get_number_of_starving_pops() const;
    float get_unemployment_rate() const;

    // Country to province mapping
    void add_province_to_country(Province* prov, int country_id);
    Dictionary get_countries_provinces(int country_id) const;
    std::unordered_set<int> get_country_provinces(int country_id) const;

    void month_tick();
    void thread_processor();
};
