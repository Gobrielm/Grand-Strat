#pragma once

#include <godot_cpp/classes/object.hpp>
#include "utility/vector2i_hash.hpp"
#include "classes/map_objects/town.hpp"

#include <mutex>
#include <shared_mutex>
#include <unordered_set>
#include <vector>
#include <map>

class BasePop;
class Factory;
class Station;
class TradingSystem;
class MarketComponent;
class EmployerComponent;
class SubsistenceFarm;
class PopManager;
class InitialBuilder;
class ProvinceManager;
class PositionComponent;
class MapObjectInfo;

enum class PopTypes: int;

using namespace godot;

class Province : public Object {
    GDCLASS(Province, Object);
    friend TradingSystem;
    friend MarketComponent;
    friend ProvinceManager;
    friend EmployerComponent;
    friend PopManager;
    friend InitialBuilder;
    friend MapObjectInfo;

    mutable std::mutex m;
    mutable std::shared_mutex pops_lock;
    int province_id;
    int country_id = -1;
    int population;
    std::vector<Vector2i> tiles;

    std::unordered_set<int> pops; // Shift ownership eventually to province


    // New Structures
    
    // Used to access the top level building on each tile
    std::unordered_map<Vector2i, PositionComponent, godot_helpers::Vector2iHasher, godot_helpers::Vector2iEqual> position_components;
    // Used to access the hidden buildings at each tile, ie sub. farms or town factories
    std::unordered_map<Vector2i, std::vector<PositionComponent>, godot_helpers::Vector2iHasher, godot_helpers::Vector2iEqual> hidden_position_components;

    // Owned objects
    Town town;

    std::unordered_map<int, std::pair<int, BuildingType>> id_to_vector_position;
    std::vector<Factory> factories;
    std::vector<Station> stations;
    std::vector<SubsistenceFarm> sub_farms;

    // private helper functions
    Factory& get_factory_unsafe(int pos_id);
    Station& get_station_unsafe(int pos_id);
    PositionComponent get_visible_position_component_unsafe(Vector2i tile) const;
    std::pair<CapitalComponent*, StorageComponent*> get_capital_and_storage_components_unsafe(const TradeOrder& order);

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

    void create_town();

    Factory& add_factory(Factory& factory);
    Factory& add_hidden_factory(Factory& factory);
    Station& add_station(Station& station);
    SubsistenceFarm& add_subsistence_farm(SubsistenceFarm& farm);

    Factory& get_factory(int pos_id);
    Station& get_station(int pos_id);
    Town& get_town();

    bool is_building_at_pos(Vector2i pos) const;
    int get_hidden_buildings_at_pos(Vector2i pos) const;

    BuildingType get_building_type(int pos_id) const;
    BuildingType get_visible_building_type(Vector2i tile) const;

    PositionComponent get_visible_position_component(Vector2i tile) const;


    // void add_terminal(Vector2i tile);
    // void remove_terminal(Vector2i tile);
    // Array get_terminal_tiles() const;
    bool has_town() const;
    Vector2i get_town_tile() const;

    void init_province();

    // === Pops ===
    // Info Stuff
    int get_number_of_pops() const;
    std::unordered_map<PopTypes, size_t> get_pop_type_statistics() const;

    // Local Pop Stuff
    void create_pops();
    void create_peasant_pop(Variant culture, Vector2i p_location);
    void create_rural_pop(Variant culture, Vector2i p_location);
    void create_town_pops(int amount);
    int create_town_pop(Variant culture, Vector2i p_location);
    void create_buildings_for_peasants();
    void employ_peasants();
    int count_pops() const;
    
};