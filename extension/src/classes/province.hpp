#pragma once

#include <godot_cpp/classes/object.hpp>
#include <shared_mutex>
#include <unordered_set>
#include "../utility/vector2i_hash.hpp"
#include "factory_template.hpp"
#include <vector>
#include <map>

class BasePop;
class Town;
class Factory;
class Station;
enum PopTypes;

using namespace godot;

class Province : public Object {
    GDCLASS(Province, Object);

    mutable std::shared_mutex m;
    mutable std::shared_mutex pops_lock;
    int province_id;
    int country_id = -1;
    int population;
    std::vector<Vector2i> tiles;
    std::unordered_set<Vector2i, godot_helpers::Vector2iHasher> terminal_tiles;

    std::vector<int> factory_ids;

    std::unordered_map<Vector2i, Vector2i, godot_helpers::Vector2iHasher> closest_town_to_tile;

    std::unordered_set<int> pops;


    // New Structures
    std::unordered_map<Vector2i, std::vector<PositionComponent&>, godot_helpers::Vector2iHasher> position_components;

    // Owned objects
    std::unordered_map<int, Town> towns;
    std::unordered_map<int, Factory> factories;
    std::unordered_map<int, Station> stations;

    protected:
    static void _bind_methods();

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
    std::unordered_map<int, float> get_demand_for_needed_goods() const;
    float get_theoretical_supply_of_grain_from_peasants() const;
    float get_demand_for_cargo(int type) const;
    void add_population(int population_to_add);
    void set_population(int new_population);
    int get_province_id() const;
    int get_country_id() const;
    void set_country_id(int p_country_id);
    Array get_tiles() const;
    const std::vector<Vector2i> get_tiles_vector() const;
    std::vector<Vector2i> get_town_centered_tiles() const;
    Vector2i get_random_tile() const;

    void add_factory(Factory& factory);
    void add_town(Town& town);
    void add_station(Station& station);

    Factory& get_factory(int pos_id);
    Town& get_town(int pos_id);
    Station& get_station(int pos_id);

    std::unordered_map<int, Factory>& get_factories() const;


    // void add_terminal(Vector2i tile);
    // void remove_terminal(Vector2i tile);
    // Array get_terminal_tiles() const;
    bool has_town() const;
    const std::unordered_set<Vector2i, godot_helpers::Vector2iHasher>& get_terminal_tiles_set() const;

    void init_province();

    //Town Stuff
    void refresh_closest_town_to_tile();
    Vector2i get_closest_town_to_tile(Vector2i tile, std::vector<Vector2i> towns);
    std::vector<Vector2i> get_town_tiles() const;

    // === Pops ===
    // Info Stuff
    int get_number_of_pops() const;
    std::unordered_map<PopTypes, size_t> get_pop_type_statistics() const;

    // Local Pop Stuff
    void create_pops();
    void create_peasant_pop(Variant culture, Vector2i p_location);
    void create_rural_pop(Variant culture, Vector2i p_location);
    void create_town_pops(int amount, const std::vector<Vector2i>& towns);
    int create_town_pop(Variant culture, Vector2i p_location);
    std::vector<int> create_buildings_for_peasants();
    void employ_peasants();
    int count_pops() const;
    Vector2i get_closest_town_tile_to_pop(const Vector2i& pop_location) const;
    bool has_closest_town_tile_to_pop(const Vector2i& pop_location) const;
    
};