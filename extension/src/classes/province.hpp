#pragma once

#include <godot_cpp/classes/object.hpp>
#include <shared_mutex>
#include <unordered_set>
#include "../utility/vector2i_hash.hpp"
#include <vector>

class BasePop;
class Terminal;
class FactoryTemplate;
class Town;
enum PopTypes;

using namespace godot;

class Province : public Object {
    GDCLASS(Province, Object);

    mutable std::mutex m;
    mutable std::shared_mutex pops_lock;
    int province_id;
    int country_id = -1;
    int population;
    std::vector<Vector2i> tiles;
    std::unordered_set<Vector2i, godot_helpers::Vector2iHasher> terminal_tiles; //Doesn't 'own' Terminals yet
    std::unordered_map<Vector2i, Vector2i, godot_helpers::Vector2iHasher> closest_town_to_tile;

    std::unordered_map<PopTypes, std::unordered_set<int>> pop_types;
    std::unordered_map<int, BasePop*> pops;

    protected:
    static void _bind_methods();
    BasePop* get_pop(int pop_id);
    
    

    public:
    //Godot Facing
    static Province* create(int p_prov_id = -1);
    void initialize(int p_prov_id = -1);

    Province();
    Province(int p_prov_id);
    ~Province();


    //Godot Facing
    void add_tile(Vector2i coords);
    int get_population() const;
    int get_demand_for_grain() const; // Used exclusively for initial builder
    void add_population(int population_to_add);
    void set_population(int new_population);
    int get_province_id() const;
    int get_country_id() const;
    void set_country_id(int p_country_id);
    Array get_tiles() const;
    const std::vector<Vector2i> get_tiles_vector() const;
    std::vector<Vector2i> get_town_centered_tiles() const;
    Vector2i get_random_tile() const;
    void add_terminal(Vector2i tile);
    void remove_terminal(Vector2i tile);
    Array get_terminal_tiles() const;
    bool has_town() const;
    const std::unordered_set<Vector2i, godot_helpers::Vector2iHasher>& get_terminal_tiles_set() const;

    //Town Stuff
    void refresh_closest_town_to_tile();
    Vector2i get_closest_town_to_tile(Vector2i tile, std::vector<Vector2i> towns);
    std::vector<Vector2i> get_town_tiles() const;

    // === Pops ===
    // Info Stuff
    int get_number_of_pops() const;
    double get_total_wealth_of_pops() const;
    float get_needs_met_of_pops() const;
    int get_number_of_broke_pops() const;
    int get_number_of_starving_pops() const;
    int get_number_of_unemployed_pops() const;
    int get_number_of_peasants() const;
    int get_number_of_city_pops() const;


    // Global pop functions
    void pay_pop(int pop_id, float wage);
    void fire_pop(int pop_id);
    void sell_cargo_to_pop(int pop_id, int type, int amount, float price);
    int get_pop_desired(int pop_id, int type, float price);

    // Local Pop Stuff
    void create_pops();
    void create_peasant_pop(Variant culture, Vector2i p_location);
    void create_rural_pop(Variant culture, Vector2i p_location);
    int create_town_pop(Variant culture, Vector2i p_location);
    void create_town_pops(int amount, const std::vector<Vector2i>& towns);
    std::vector<int> create_buildings_for_peasants();
    void employ_peasants();
    int count_pops() const;
    void find_employment_for_pops();
    void find_employment_for_town_pops();
    void find_employment_for_rural_pops();
    Ref<FactoryTemplate> find_rural_employment(BasePop* pop) const;
    Ref<FactoryTemplate> find_urban_employment(BasePop* pop) const;
    void month_tick();
    void change_pops();
    void sell_to_pops();
    

    
};