#pragma once
#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/classes/tile_map_layer.hpp>

#include <godot_cpp/variant/dictionary.hpp>
#include <godot_cpp/variant/array.hpp>

#include <mutex>
#include <shared_mutex>
#include <thread>
#include <condition_variable>

#include "terminal_map_utility/terminal_map_thread_pool.hpp"
#include "../utility/vector2i_hash.hpp"
#include "../classes/construction_site.hpp"

class Terminal;
class Broker;
class Factory;
class Town;
class StationWOMethods;
class RoadDepot;


using namespace godot;

class TerminalMap : public RefCounted {
    GDCLASS(TerminalMap, RefCounted);

private:
    static Ref<TerminalMap> singleton_instance;

    TileMapLayer* map = nullptr;
    TileMapLayer* cargo_map = nullptr;
    Node2D* cargo_values = nullptr;
    Node* cargo_controller = nullptr;

    mutable std::shared_mutex cargo_map_mutex;  // Protected cargo_map_terminals and terminal_id_to_terminal exclusively
    mutable std::mutex m;                       // Protects everyhing else, mainly the different godot objects above
    std::unordered_map<Vector2i, int, godot_helpers::Vector2iHasher> cargo_map_terminals;
    std::unordered_map<int, Ref<Terminal>> terminal_id_to_terminal;

    TerminalMapThreadPool* thread_pool = nullptr;

protected:
    static void _bind_methods();

public:
    static void initialize_singleton(TileMapLayer* p_map);

    TerminalMap();
    ~TerminalMap();

    static bool is_instance_created();
    static Ref<TerminalMap> get_instance();
    void assign_cargo_map(TileMapLayer* p_cargo_map);
    void assign_cargo_controller(Node* p_cargo_controller);

    //Process hooks
    void _on_day_tick_timeout();
    void _on_month_tick_timeout();

    //Process hook utility
    std::vector<Ref<Terminal>> get_terminals_for_day_tick() const;
    std::vector<Ref<Terminal>> get_terminals_for_month_tick() const;

    void clear();
    TileMapLayer* get_main_map() const;
    TileMapLayer* get_cargo_map() const;

    //Time
    void pause_time();
    void unpause_time();

    //Creators
    void create_isolated_terminal(Ref<Terminal> p_terminal);
    void create_terminal(Ref<Terminal> p_terminal);
    void encode_factory(Ref<Factory> factory, int mult = 1);
    void encode_factory_from_construction_site(Ref<Factory> factory);
    void add_connected_brokers(Ref<Broker> p_broker);
    void add_connected_stations(Ref<RoadDepot> road_depot);
    void find_stations(Ref<Broker> broker);
    Ref<Factory> create_factory(const Vector2i &p_location, int p_player_owner, const Dictionary &p_inputs, const Dictionary &p_outputs);
    Ref<Factory> create_primary_factory(const Vector2i &p_location, int p_player_owner, int type) const;
    
    //Checkers
    int get_cargo_value_of_tile(const Vector2i &coords, int type) const;
    std::vector<int> get_available_resources_of_tile(const Vector2i &coords) const;
    bool is_terminal(const Vector2i &coords);
    bool is_hold(const Vector2i &coords);
    bool is_owned_recipeless_construction_site(const Vector2i &coords);
    bool is_building(const Vector2i &coords);
    bool is_owned_building(const Vector2i &coords, int id);
    bool is_owned_construction_site(const Vector2i &coords);
    bool is_factory(const Vector2i &coords);
    bool is_station(const Vector2i &coords);
    bool is_road_depot(const Vector2i &coords);
    bool is_owned_station(const Vector2i &coords, int player_id);
    bool is_ai_station(const Vector2i &coords);
    bool is_owned_ai_station(const Vector2i &coords, int id);
    bool is_town(const Vector2i &coords);

    //Info getters
    Dictionary get_cargo_dict(const Vector2i &coords);
    Array get_construction_site_recipe(const Vector2i &coords);
    Dictionary get_construction_materials(const Vector2i &coords);
    int get_cash_of_firm(const Vector2i &coords);
    Dictionary get_local_prices(const Vector2i &coords);
    Dictionary get_station_orders(const Vector2i &coords);
    Dictionary get_town_fulfillment(const Vector2i &coords);
    bool is_tile_traversable(const Vector2i& coords, bool is_water_untraversable = true);
    bool is_tile_available(const Vector2i& coords);
    Array get_available_primary_recipes(const Vector2i& coords) const;

    //Getters
    Ref<Terminal> get_terminal(const Vector2i &coords);
    Ref<Broker> get_broker(const Vector2i &coords);
    Ref<StationWOMethods> get_station(const Vector2i &coords);
    Ref<StationWOMethods> get_ai_station(const Vector2i &coords);
    Ref<Town> get_town(const Vector2i &coords);
    Ref<Factory> get_factory(const Vector2i &coords);

    template <typename T>
    Ref<T> get_terminal_as(const Vector2i &coords, const std::function<bool(const Vector2i &)> &type_check = nullptr) const {
        Ref<T> toReturn = Ref<T>(nullptr);
        {
            std::shared_lock lock(cargo_map_mutex);
            auto it = cargo_map_terminals.find(coords);
            if (it != cargo_map_terminals.end()) {
                auto it2 = terminal_id_to_terminal.find(it->second);
                if (it2 != terminal_id_to_terminal.end() && (!type_check || type_check(coords))) {
                    Ref<T> typed = it2->second;
                    if (typed.is_valid()) {
                        toReturn = typed;
                    }
                }
            }
        }
        return toReturn;
    }

    template <typename T>
    Ref<T> get_terminal_as(int terminal_id) const {
        Ref<T> toReturn = Ref<T>(nullptr);
        {
            std::shared_lock lock(cargo_map_mutex);
            if (terminal_id_to_terminal.count(terminal_id)) {
                Ref<T> typed = terminal_id_to_terminal.at(terminal_id);
                if (typed.is_valid()) {
                    toReturn = typed;
                }
            }
        }
        return toReturn;
    }
   
    //Action doers
    void set_construction_site_recipe(const Vector2i &coords, const Array &selected_recipe);
    void destroy_recipe(const Vector2i &coords);
    void transform_construction_site_to_factory(const Vector2i &coords);
    void edit_order_station(const Vector2i &coords, int type, int amount, bool buy, float max_price);
    void remove_order_station(const Vector2i &coords, int type);
    void refresh_road_depots(const std::unordered_set<Vector2i, godot_helpers::Vector2iHasher> &s);

    //Economy Testing Functions
    float get_average_cash_of_road_depot() const;
    float get_average_cash_of_factory() const;
    float get_average_factory_level() const;
    int get_grain_demand() const;
    int get_grain_supply() const;

    template <typename T>
    float get_average_cash_of_terminal() const;
};

