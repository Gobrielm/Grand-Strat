#pragma once
#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/classes/tile_map_layer.hpp>

#include <godot_cpp/variant/dictionary.hpp>
#include <godot_cpp/variant/array.hpp>

#include <mutex>
#include <thread>

#include "../classes/terminal.hpp"
#include "../classes/station.hpp"
#include "../classes/broker.hpp"
#include "../classes/construction_site.hpp"
#include "../classes/Factory.hpp"
#include "../classes/ai_factory.hpp"
#include "../classes/town.hpp"
#include "../classes/road_depot_wo_methods.hpp"

using namespace godot;

class TerminalMap : public RefCounted {
    GDCLASS(TerminalMap, RefCounted);

private:
    static Ref<TerminalMap> singleton_instance;

    TileMapLayer* map = nullptr;
    TileMapLayer* cargo_map = nullptr;

    std::mutex m;
    std::unordered_map<Vector2i, std::mutex*, godot_helpers::Vector2iHasher> object_mutexs;
    std::unordered_map<Vector2i, Ref<Terminal>, godot_helpers::Vector2iHasher> cargo_map_terminals;

    std::vector<std::thread> month_threads;
    bool day_tick_priority = false;

    using MapType = std::unordered_map<Vector2i, Ref<Terminal>, godot_helpers::Vector2iHasher>;
    void _on_month_tick_timeout_helper(MapType::iterator start, MapType::iterator end);

protected:
    static void _bind_methods();

public:
    static void initialize_singleton(TileMapLayer* p_map);

    TerminalMap(TileMapLayer* p_map = nullptr);
    ~TerminalMap();

    static Ref<TerminalMap> get_instance();
    void assign_cargo_map(TileMapLayer* p_cargo_map);

    //Process hooks
    void _on_day_tick_timeout();
    void _on_month_tick_timeout();

    void clear();
    TileMapLayer* get_main_map() const;

    //Creators
    void create_terminal(Ref<Terminal> p_terminal);
    void add_connected_brokers(Ref<Broker> p_broker);
    Ref<Factory> create_factory(const Vector2i &p_location, int p_player_owner, const Dictionary &p_inputs, const Dictionary &p_outputs);
    
    //Checkers
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

    //External Locks
    void lock(const Vector2i& coords); //If it returns false, then the terminal does not exist, Error
    void unlock(const Vector2i& coords);

    //Getters
    Ref<Terminal> get_terminal(const Vector2i &coords);
    Ref<Broker> get_broker(const Vector2i &coords);
    Ref<StationWOMethods> get_station(const Vector2i &coords);
    Ref<StationWOMethods> get_ai_station(const Vector2i &coords);
    Ref<Town> get_town(const Vector2i &coords);

    template <typename T>
    Ref<T> get_terminal_as(const Vector2i &coords, const std::function<bool(const Vector2i &)> &type_check = nullptr);
   

    //Action doers
    void set_construction_site_recipe(const Vector2i &coords, const Array &selected_recipe);
    void destroy_recipe(const Vector2i &coords);
    void transform_construction_site_to_factory(const Vector2i &coords);
    void edit_order_station(const Vector2i &coords, int type, int amount, bool buy, float max_price);
    void remove_order_station(const Vector2i &coords, int type);
};

