#pragma once

#include <unordered_map>
#include <mutex>
#include <shared_mutex>
#include <condition_variable>
#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/variant/dictionary.hpp>
#include <godot_cpp/variant/array.hpp>

#include "classes/province.hpp"

class BlockingThreadPool;

using namespace godot;

class ProvinceManager : public RefCounted {
    GDCLASS(ProvinceManager, RefCounted);

    mutable std::shared_mutex province_mutex;
    static Ref<ProvinceManager> singleton_instance;

    BlockingThreadPool* thread_pool;

    std::unordered_map<int, int> province_id_to_vector_position; // Province id -> province
    std::vector<Province*> provinces;

    std::unordered_map<Vector2i, int, godot_helpers::Vector2iHasher> tiles_to_province_id; 
    std::unordered_map<int, std::unordered_set<int>> country_id_to_province_ids;

    Province* get_province_private(int province_id);

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
    void finish_province_creation(int province_id);

    // Population handling
    void add_population_to_province(Vector2i tile, int pop);
    int get_province_population(Vector2i tile);
    int get_population(int province_id);
    int get_population_as_level(int province_id);
    int get_total_population() const;
    int get_number_of_pops_in_country(int country_id) const;
    void create_pops();

    void create_pops_range(std::vector<Province*>::iterator start, std::vector<Province*>::iterator end);

    // Province queries
    std::vector<int> get_provinces_vector();
    Array get_provinces() const;
    bool is_tile_a_province(Vector2i tile) const;
    int get_province_id(Vector2i tile) const;
    int get_province_id_unsafe(Vector2i tile) const;
    Province* get_province_godot(int province_id) const;
    Province* get_province(int province_id) const;
    Province* get_province(const Vector2i& tile) const;

    bool is_tile_available(const Vector2i coords);

    // Country to province mapping
    void add_province_to_country(Province* prov, int country_id);
    Dictionary get_countries_provinces(int country_id) const;
    std::unordered_set<int> get_country_provinces(int country_id) const;
    std::unordered_set<int> get_country_ids() const;

    //Stats Stuff
    // std::unordered_map<int, float> get_average_country_prices(int country_id) const;

    // Godot Map Object Functions
    bool is_factory(Vector2i tile);
    bool is_town(Vector2i tile);
    bool is_station(Vector2i tile);

    Array get_town_factories(Vector2i town_tile);
    Dictionary get_town_pdps(Vector2i town_tile);
    Dictionary get_town_pdhs(Vector2i town_tile);

    Dictionary get_factory_info(Vector2i coords);
    Dictionary get_subsistence_farm_info(Vector2i coords);
    int16_t get_cash_of_factory(Vector2i coords);

    //Economy Testing Functions
    float get_average_cash_of_sub_farms() const;
    float get_average_cash_of_factory() const;
    float get_average_cash_of_station() const;
    float get_average_factory_level() const;
    unsigned long get_grain_demand() const;
    unsigned long get_grain_supply() const;
    float get_average_price(int type) const;

    void pay_pops(int num_to_pay, double for_each);

    void test_check();

    // Simulation
    void simulation_tick();

    // Orders
    void order_tick();

    // Bookkeeping
    void bookkeeping_tick();

    

    // Trading
    void trading_tick();
};
