#include "terminal_map.hpp"

using namespace godot;

Ref<TerminalMap> TerminalMap::singleton_instance = nullptr;

void TerminalMap::_bind_methods() {
    ClassDB::bind_static_method(get_class_static(), D_METHOD("get_instance"), &TerminalMap::get_instance);
    ClassDB::bind_static_method(get_class_static(), D_METHOD("create", "p_map"), &TerminalMap::create);
    // You should continue binding all necessary methods here
}

Ref<TerminalMap> TerminalMap::create(TileMapLayer* p_map) { return memnew(TerminalMap(p_map)); }

TerminalMap::TerminalMap(TileMapLayer* p_map) {
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

TerminalMap::~TerminalMap() {
    //Clean up old threads
    for (auto &thread: month_threads) {
        thread.join();
    }
    month_threads.clear();
    for (auto &[__, mtx]: object_mutexs) {
        delete (mtx);
    }
}

Ref<TerminalMap> TerminalMap::get_instance() { 
    ERR_FAIL_COND_V(singleton_instance == nullptr, nullptr);
    return singleton_instance;
}

void TerminalMap::assign_cargo_map(TileMapLayer* p_cargo_map) {}

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

void TerminalMap::_on_month_tick_timeout() {
    //Clean up old threads
    for (auto &thread: month_threads) {
        thread.join();
    }
    month_threads.clear();

    auto start = cargo_map_terminals.begin();

    const int NUMBER_OF_THREADS = 4;
    const int chunk_size = cargo_map_terminals.size() / NUMBER_OF_THREADS;

    for (int i = 0; i < NUMBER_OF_THREADS; i++) {
        auto end = start;
        int step = chunk_size + (i == NUMBER_OF_THREADS - 1 ? cargo_map_terminals.size() % NUMBER_OF_THREADS : 0);
        std::advance(end, step);
        
        // std::thread thrd([this, start, end]() {
        //     _on_month_tick_timeout_helper(IteratorPair(start, end));
        // });
        // month_threads.push_back(thrd);
        start = end;
    }
}

void TerminalMap::_on_month_tick_timeout_helper(const IteratorPair &range) {
    for (auto it = range.first; it != range.second; it++) {
        while (day_tick_priority) {
            std::this_thread::sleep_for(std::chrono::milliseconds(2));
        }
        Vector2i coords = it -> first;
        object_mutexs[coords]->lock();
        Terminal* terminal = it -> second;
        
        if (terminal->has_method("month_tick")) {
            terminal->call("month_tick");
        }
        object_mutexs[coords]->unlock();
    }
}

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

void TerminalMap::add_connected_brokers(Broker *p_broker) {
    Array connected = map->get_surrounding_cells(p_broker->get_location());
    for (int i = 0; i < connected.size(); ++i) {
        Vector2i tile = connected[i];
        Broker *other = get_broker(tile);
        if (!other) continue;
        p_broker->add_connected_broker(other);
        other->add_connected_broker(p_broker);
    }
}

Factory* TerminalMap::create_factory(const Vector2i &p_location, int p_player_owner, const Dictionary &p_inputs, const Dictionary &p_outputs) {
    if (p_player_owner > 0) {
        return memnew(Factory(p_location, p_player_owner, p_inputs, p_outputs));
    }
    return memnew(AiFactory(p_location, p_player_owner, p_inputs, p_outputs));
}

void TerminalMap::create_ai_station(const Vector2i &coords, int orientation, int p_owner) {} //TODO: 


//Checkers
bool TerminalMap::is_tile_taken(const Vector2i &coords) { //Add Utils call
    m.lock();
    bool toReturn = cargo_map_terminals.count(coords);
    m.unlock();
    return toReturn;
}
bool TerminalMap::is_hold(const Vector2i &coords) {
    return dynamic_cast<Hold*>(get_terminal(coords)) != nullptr;
}
bool TerminalMap::is_owned_recipeless_construction_site(const Vector2i &coords) {
    bool toReturn = false;
    object_mutexs[coords] -> lock();
    ConstructionSite* construction_site = dynamic_cast<ConstructionSite*>(cargo_map_terminals[coords]);
    if (construction_site) toReturn = construction_site -> has_recipe();
    object_mutexs[coords] -> unlock();
    return toReturn;
}
bool TerminalMap::is_building(const Vector2i &coords) {
    bool toReturn = false;
    object_mutexs[coords]->lock();
    toReturn = dynamic_cast<ConstructionSite*>(cargo_map_terminals[coords]) || dynamic_cast<Town*>(cargo_map_terminals[coords]) || dynamic_cast<FactoryTemplate*>(cargo_map_terminals[coords]);
    object_mutexs[coords]->unlock();
    return toReturn;
}
bool TerminalMap::is_owned_building(const Vector2i &coords, int id) {
    if (is_building(coords)) {
        return cargo_map_terminals[coords] -> get_player_owner() == id;
    }
    return false;
}
bool TerminalMap::is_owned_construction_site(const Vector2i &coords) {
    return dynamic_cast<ConstructionSite*>(get_terminal(coords)) != nullptr;
}
bool TerminalMap::is_factory(const Vector2i &coords) {
    return dynamic_cast<Factory*>(get_terminal(coords)) != nullptr;
}
bool TerminalMap::is_station(const Vector2i &coords) {
    return dynamic_cast<StationWOMethods*>(get_terminal(coords)) != nullptr;
}
bool TerminalMap::is_road_depot(const Vector2i &coords) {
    return dynamic_cast<RoadDepotWOMethods*>(get_terminal(coords)) != nullptr;
}
bool TerminalMap::is_owned_station(const Vector2i &coords, int player_id) {
    StationWOMethods* temp = dynamic_cast<StationWOMethods*>(get_terminal(coords));
    if (temp) {
        return temp->get_player_owner() == player_id;
    }
    return false;
}
bool TerminalMap::is_ai_station(const Vector2i &coords) {//TODO
    return false;
} 
bool TerminalMap::is_owned_ai_station(const Vector2i &coords, int id) {//TODO
    return false;
} 
bool TerminalMap::is_town(const Vector2i &coords) {
    return dynamic_cast<Town*>(get_terminal(coords)) != nullptr;
}

//Info getters
Array TerminalMap::get_construction_site_recipe(const Vector2i &coords) {
    Array toReturn = {};
    Terminal* temp = get_terminal(coords);
    object_mutexs[coords]->lock();
    if (ConstructionSite* construction_site = dynamic_cast<ConstructionSite*>(temp)) {
        toReturn = construction_site->get_recipe();
    }
    object_mutexs[coords]->unlock();
    return toReturn;
}

Dictionary TerminalMap::get_construction_materials(const Vector2i &coords) {
    Dictionary toReturn;
    if (is_owned_construction_site(coords)) {
        object_mutexs[coords] -> lock();
        toReturn = dynamic_cast<ConstructionSite*>(cargo_map_terminals[coords]) -> get_construction_materials();
        object_mutexs[coords] -> unlock();
    }
    return toReturn;
}

int TerminalMap::get_cash_of_firm(const Vector2i &coords) {
    int toReturn;
    object_mutexs[coords] -> lock();
    if (Firm* firm = dynamic_cast<Firm*>(cargo_map_terminals[coords])) {
        toReturn = firm->get_cash();
    }
    object_mutexs[coords] -> unlock();
    return toReturn;
}

Dictionary TerminalMap::get_local_prices(const Vector2i &coords) {
    Dictionary toReturn;
    Broker* broker = get_broker(coords);
    if (broker) {
        object_mutexs[coords] -> lock();
        toReturn = broker -> get_local_prices();
        object_mutexs[coords] -> unlock();
    }
    return toReturn;
}

Dictionary TerminalMap::get_station_orders(const Vector2i &coords) {
    Dictionary toReturn;
    if (is_station(coords)) {
        object_mutexs[coords] -> lock();
        toReturn = dynamic_cast<StationWOMethods*>(cargo_map_terminals[coords]) -> get_orders_dict();
        object_mutexs[coords] -> unlock();
    }
    return toReturn;
}

Array TerminalMap::get_available_primary_recipes(const Vector2i &coords) { //TODO
    return Array();
} 

Dictionary TerminalMap::get_town_fulfillment(const Vector2i &coords) {
    Dictionary toReturn;
    Town* town = get_town(coords);
    if (town) {
        object_mutexs[coords] -> lock();
        toReturn = town -> get_fulfillment_dict();
        object_mutexs[coords] -> unlock();
    }
    return toReturn;
}


//Internal Getters
Terminal* TerminalMap::get_terminal(const Vector2i &coords) {
    return cargo_map_terminals[coords];
}
Broker* TerminalMap::get_broker(const Vector2i &coords) {
    if (Broker* broker = dynamic_cast<Broker*>(cargo_map_terminals[coords])) {
        return broker;
    }
    return nullptr;
}
StationWOMethods* TerminalMap::get_station(const Vector2i &coords) {
    if (StationWOMethods* station = dynamic_cast<StationWOMethods*>(cargo_map_terminals[coords])) {
        return station;
    }
    return nullptr;
}
Town* TerminalMap::get_town(const Vector2i &coords) {
    if (Town* town = dynamic_cast<Town*>(cargo_map_terminals[coords])) {
        return town;
    }
    return nullptr;
}


//External Getters
ScopedTerminal* TerminalMap::request_terminal(const Vector2i &coords) {
    object_mutexs[coords]->lock();
    return memnew(ScopedTerminal(get_terminal(coords)));
}

void TerminalMap::return_terminal(ScopedTerminal* scoped_terminal) {
    Terminal* terminal = scoped_terminal -> get_value();
    if (terminal) {
        Vector2i coords = terminal -> get_location();
        object_mutexs[coords]->unlock();
        scoped_terminal -> release_lock();
    }
}

//Action doers
void TerminalMap::set_construction_site_recipe(const Vector2i &coords, const Array &selected_recipe) {
    if (is_owned_recipeless_construction_site(coords)) {
        object_mutexs[coords] -> lock();
        dynamic_cast<ConstructionSite*>(cargo_map_terminals[coords]) -> set_recipe(selected_recipe);
        object_mutexs[coords] -> unlock();
    }
}

void TerminalMap::destroy_recipe(const Vector2i &coords) {
    if (is_owned_recipeless_construction_site(coords)) {
        object_mutexs[coords] -> lock();
        dynamic_cast<ConstructionSite*>(cargo_map_terminals[coords]) -> destroy_recipe();
        object_mutexs[coords] -> unlock();
    }
}

void TerminalMap::transform_construction_site_to_factory(const Vector2i &coords) { //TODO
    if (is_owned_construction_site(coords)) {
        object_mutexs[coords] -> lock();
        ConstructionSite* old_site = dynamic_cast<ConstructionSite*>(cargo_map_terminals[coords]);
        
        cargo_map_terminals[coords] = create_factory(coords, old_site->get_player_owner(), old_site->get_recipe()[0], old_site->get_recipe()[1]);

        memdelete(old_site);
        object_mutexs[coords] -> unlock();
        // cargo_map.transform_construction_site_to_factory(coords)
    }
}

void TerminalMap::edit_order_station(const Vector2i &coords, int type, int amount, bool buy, float max_price) {
    if (is_station(coords)) {
        object_mutexs[coords] -> lock();

        StationWOMethods* station = dynamic_cast<StationWOMethods*>(cargo_map_terminals[coords]);
        station -> edit_order(type, amount, buy, max_price);

        object_mutexs[coords] -> unlock();
    }
}

void TerminalMap::remove_order_station(const Vector2i &coords, int type) {
    if (is_station(coords)) {
        object_mutexs[coords] -> lock();

        StationWOMethods* station = dynamic_cast<StationWOMethods*>(cargo_map_terminals[coords]);
        station -> remove_order(type);

        object_mutexs[coords] -> unlock();
    }
}