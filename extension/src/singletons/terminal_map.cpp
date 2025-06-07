#include "terminal_map.hpp"

using namespace godot;

Ref<TerminalMap> TerminalMap::singleton_instance = nullptr;

void TerminalMap::_bind_methods() {
    ClassDB::bind_static_method(get_class_static(), D_METHOD("get_instance"), &TerminalMap::get_instance);
    ClassDB::bind_static_method(get_class_static(), D_METHOD("create", "p_map"), &TerminalMap::create);
    // You should continue binding all necessary methods here
}

Ref<TerminalMap> TerminalMap::create(Ref<TileMapLayer> p_map) { return memnew(TerminalMap(p_map)); }

TerminalMap::TerminalMap(Ref<TileMapLayer> p_map) {
    singleton_instance.instantiate();
    singleton_instance -> map = p_map;
    Dictionary needs;


    Ref<CargoInfo> cargo_info = CargoInfo::get_instance();
    needs[cargo_info -> get_cargo_type("grain")] = 1;
    needs[cargo_info -> get_cargo_type("wood")] = 0.3;
    needs[cargo_info -> get_cargo_type("salt")] = 0.1;
    needs[cargo_info -> get_cargo_type("fish")] = 0.2;
    needs[cargo_info -> get_cargo_type("fruit")] = 0.2;
    needs[cargo_info -> get_cargo_type("meat")] = 0.2;
    needs[cargo_info -> get_cargo_type("bread")] = 0.3;
    needs[cargo_info -> get_cargo_type("clothes")] = 0.3;
    needs[cargo_info -> get_cargo_type("furniture")] = 0.3;
    BasePop::create_base_needs(needs);
}

Ref<TerminalMap> TerminalMap::get_instance() { 
    ERR_FAIL_COND_V(singleton_instance == nullptr, nullptr);
    return singleton_instance;
}

void TerminalMap::assign_cargo_map(Ref<TileMapLayer> p_cargo_map) {}

//Process hooks
void TerminalMap::_on_day_tick_timeout() {
    m.lock();
    day_tick_priority = true;
    m.unlock();

    for (const auto &[coords, terminal]: cargo_map_terminals) {
        std::mutex* obj_mutex = object_mutexs[coords];
        obj_mutex->lock();
        if (terminal->has_method("day_tick")) {
            terminal->call("day_tick");
        }
        obj_mutex->unlock();
    }

    m.lock();
    day_tick_priority = false;
    m.unlock();
}



// void TerminalMap::_on_month_tick_timeout() { TODO: Use iterators

// }

// void TerminalMap::_on_month_tick_timeout_helper(int from, int to) {
//     for (int i = from; i < to; ++i) {
//         while (day_tick_priority) {
//             std::this_thread::sleep_for(std::chrono::milliseconds(2));
//         }
//         Vector2i coords = cargo_map_terminals[i];
//         Terminal *obj = Object::cast_to<Terminal>(cargo_map_terminals[coords]);
//         Mutex *obj_mutex = Object::cast_to<Mutex>(object_mutexs[coords]);
//         obj_mutex->lock();
//         if (obj->has_method("month_tick")) {
//             obj->call("month_tick");
//         }
//         obj_mutex->unlock();
//     }
// }

void TerminalMap::clear() {
    m.lock();
    cargo_map_terminals.clear();
    object_mutexs.clear();
    m.unlock();
}

//Resources
// Dictionary TerminalMap::get_available_resources(const Vector2i &coords); // Switch to cargo_map

//Creators
void TerminalMap::create_station(const Vector2i &coords, int player_id) { //TODO: DO call outside
    StationWOMethods* station = memnew(StationWOMethods(coords, player_id));
    create_terminal(station);
}

void TerminalMap::create_road_depot(const Vector2i &coords, int player_id) {
    RoadDepotWOMethods* road_depot = memnew(RoadDepotWOMethods(coords, player_id));
    create_terminal(road_depot);
}

void TerminalMap::create_terminal(Terminal *p_terminal) {
    m.lock();
    object_mutexs[p_terminal -> get_location()] = new std::mutex;
    cargo_map_terminals[p_terminal -> get_location()] = p_terminal;
    if (Broker* broker = dynamic_cast<Broker*>(p_terminal)) {
        add_connected_brokers(broker);
    }
    m.unlock();
}

void TerminalMap::add_connected_brokers(Broker *p_broker);
Factory* TerminalMap::create_factory(const Vector2i &p_location, int p_player_owner, const Dictionary &p_inputs, const Dictionary &p_outputs);
void TerminalMap::create_ai_station(const Vector2i &coords, int orientation, int p_owner);


//Checkers
bool TerminalMap::is_tile_taken(const Vector2i &coords);
bool TerminalMap::is_hold(const Vector2i &coords);
bool TerminalMap::is_owned_recipeless_construction_site(const Vector2i &coords);
bool TerminalMap::is_building(const Vector2i &coords);
bool TerminalMap::is_owned_building(const Vector2i &coords, int id);
bool TerminalMap::is_owned_construction_site(const Vector2i &coords);
bool TerminalMap::is_factory(const Vector2i &coords);
bool TerminalMap::is_station(const Vector2i &coords);
bool TerminalMap::is_road_depot(const Vector2i &coords);
bool TerminalMap::is_owned_station(const Vector2i &coords, int player_id);
bool TerminalMap::is_ai_station(const Vector2i &coords);
bool TerminalMap::is_owned_ai_station(const Vector2i &coords, int id);
bool TerminalMap::is_town(const Vector2i &coords);

//Info getters
Dictionary TerminalMap::get_hold(const Vector2i &coords);
Array TerminalMap::get_construction_site_recipe(const Vector2i &coords);
Dictionary TerminalMap::get_construction_materials(const Vector2i &coords);
int TerminalMap::get_cash_of_firm(const Vector2i &coords);
Dictionary TerminalMap::get_local_prices(const Vector2i &coords);
Dictionary TerminalMap::get_station_orders(const Vector2i &coords);
Array TerminalMap::get_available_primary_recipes(const Vector2i &coords);
Dictionary TerminalMap::get_town_fulfillment(const Vector2i &coords);

//Getters
Terminal* TerminalMap::get_terminal(const Vector2i &coords);
Broker* TerminalMap::get_broker(const Vector2i &coords);
StationWOMethods* TerminalMap::get_station(const Vector2i &coords);
Town* TerminalMap::get_town(const Vector2i &coords);

//Action doers
void TerminalMap::set_construction_site_recipe(const Vector2i &coords, const Array &selected_recipe);
void TerminalMap::destroy_recipe(const Vector2i &coords);
void TerminalMap::transform_construction_site_to_factory(const Vector2i &coords);
void TerminalMap::edit_order_station(const Vector2i &coords, int type, int amount, bool buy, float max_price);
void TerminalMap::remove_order_station(const Vector2i &coords, int type);