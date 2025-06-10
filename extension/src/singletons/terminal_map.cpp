#include "terminal_map.hpp"

using namespace godot;

Ref<TerminalMap> TerminalMap::singleton_instance = nullptr;

void TerminalMap::_bind_methods() {
    ClassDB::bind_static_method(get_class_static(), D_METHOD("get_instance"), &TerminalMap::get_instance);
    ClassDB::bind_static_method(get_class_static(), D_METHOD("create", "p_map"), &TerminalMap::initialize_singleton);
    // Initialization
    ClassDB::bind_method(D_METHOD("assign_cargo_map", "p_cargo_map"), &TerminalMap::assign_cargo_map);

    // Process hooks
    ClassDB::bind_method(D_METHOD("_on_day_tick_timeout"), &TerminalMap::_on_day_tick_timeout);
    ClassDB::bind_method(D_METHOD("_on_month_tick_timeout"), &TerminalMap::_on_month_tick_timeout);

    // Core functions
    ClassDB::bind_method(D_METHOD("clear"), &TerminalMap::clear);
    ClassDB::bind_method(D_METHOD("get_main_map"), &TerminalMap::get_main_map);

    // Creators
    ClassDB::bind_method(D_METHOD("create_terminal", "p_terminal"), &TerminalMap::create_terminal);
    ClassDB::bind_method(D_METHOD("add_connected_brokers", "p_broker"), &TerminalMap::add_connected_brokers);
    ClassDB::bind_method(D_METHOD("create_factory", "p_location", "p_player_owner", "p_inputs", "p_outputs"), &TerminalMap::create_factory);

    // Checkers
    ClassDB::bind_method(D_METHOD("is_hold", "coords"), &TerminalMap::is_hold);
    ClassDB::bind_method(D_METHOD("is_owned_recipeless_construction_site", "coords"), &TerminalMap::is_owned_recipeless_construction_site);
    ClassDB::bind_method(D_METHOD("is_building", "coords"), &TerminalMap::is_building);
    ClassDB::bind_method(D_METHOD("is_owned_building", "coords", "id"), &TerminalMap::is_owned_building);
    ClassDB::bind_method(D_METHOD("is_owned_construction_site", "coords"), &TerminalMap::is_owned_construction_site);
    ClassDB::bind_method(D_METHOD("is_factory", "coords"), &TerminalMap::is_factory);
    ClassDB::bind_method(D_METHOD("is_station", "coords"), &TerminalMap::is_station);
    ClassDB::bind_method(D_METHOD("is_road_depot", "coords"), &TerminalMap::is_road_depot);
    ClassDB::bind_method(D_METHOD("is_owned_station", "coords", "player_id"), &TerminalMap::is_owned_station);
    ClassDB::bind_method(D_METHOD("is_ai_station", "coords"), &TerminalMap::is_ai_station);
    ClassDB::bind_method(D_METHOD("is_owned_ai_station", "coords", "id"), &TerminalMap::is_owned_ai_station);
    ClassDB::bind_method(D_METHOD("is_town", "coords"), &TerminalMap::is_town);

    // Info getters

    ClassDB::bind_method(D_METHOD("get_cargo_dict", "coords"), &TerminalMap::get_cargo_dict);
    ClassDB::bind_method(D_METHOD("get_construction_site_recipe", "coords"), &TerminalMap::get_construction_site_recipe);
    ClassDB::bind_method(D_METHOD("get_construction_materials", "coords"), &TerminalMap::get_construction_materials);
    ClassDB::bind_method(D_METHOD("get_cash_of_firm", "coords"), &TerminalMap::get_cash_of_firm);
    ClassDB::bind_method(D_METHOD("get_local_prices", "coords"), &TerminalMap::get_local_prices);
    ClassDB::bind_method(D_METHOD("get_station_orders", "coords"), &TerminalMap::get_station_orders);
    ClassDB::bind_method(D_METHOD("get_town_fulfillment", "coords"), &TerminalMap::get_town_fulfillment);

    ClassDB::bind_method(D_METHOD("get_terminal", "coords"), &TerminalMap::get_terminal);
    ClassDB::bind_method(D_METHOD("get_broker", "coords"), &TerminalMap::get_broker);
    ClassDB::bind_method(D_METHOD("get_station", "coords"), &TerminalMap::get_station);
    ClassDB::bind_method(D_METHOD("get_ai_station", "coords"), &TerminalMap::get_ai_station);
    ClassDB::bind_method(D_METHOD("get_town", "coords"), &TerminalMap::get_town);

    // Action doers
    ClassDB::bind_method(D_METHOD("set_construction_site_recipe", "coords", "selected_recipe"), &TerminalMap::set_construction_site_recipe);
    ClassDB::bind_method(D_METHOD("destroy_recipe", "coords"), &TerminalMap::destroy_recipe);
    ClassDB::bind_method(D_METHOD("transform_construction_site_to_factory", "coords"), &TerminalMap::transform_construction_site_to_factory);
    ClassDB::bind_method(D_METHOD("edit_order_station", "coords", "type", "amount", "buy", "max_price"), &TerminalMap::edit_order_station);
    ClassDB::bind_method(D_METHOD("remove_order_station", "coords", "type"), &TerminalMap::remove_order_station);
}

void TerminalMap::initialize_singleton(TileMapLayer* p_map) {
    ERR_FAIL_COND_MSG(singleton_instance != nullptr, "Cannot create multiple instances of singleton!");
    singleton_instance.instantiate();
    singleton_instance -> map = p_map;
}

TerminalMap::TerminalMap(TileMapLayer* p_map) {
    map = p_map;
    Dictionary needs;


    Ref<CargoInfo> cargo_info = CargoInfo::get_instance(); //TODO: Should be expanded and moved elsewhere
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
    cargo_map_terminals.clear();
}

Ref<TerminalMap> TerminalMap::get_instance() { 
    ERR_FAIL_COND_V_MSG(singleton_instance == nullptr, nullptr, "Terminal Map has not been created yet");
    return singleton_instance;
}

void TerminalMap::assign_cargo_map(TileMapLayer* p_cargo_map) {}

//Process hooks
void TerminalMap::_on_day_tick_timeout() {
    m.lock();
    day_tick_priority = true;
    m.unlock();

    for (const auto &[coords, terminal]: cargo_map_terminals) {
        if (terminal->has_method("day_tick")) {
            terminal->call("day_tick");
        }
    }

    m.lock();
    day_tick_priority = false;
    m.unlock();
}

void TerminalMap::_on_month_tick_timeout() {
    // Clean up old threads
    for (auto &thread: month_threads) {
        thread.join();
    }
    month_threads.clear();

    auto start = cargo_map_terminals.begin();

    const int NUMBER_OF_THREADS = 4;
    const int chunk_size = cargo_map_terminals.size() / NUMBER_OF_THREADS;

    for (int i = 0; i < NUMBER_OF_THREADS; i++) {
        auto end = start;
        std::advance(end, chunk_size);
        if (i == (NUMBER_OF_THREADS - 1)) {
            end = cargo_map_terminals.end();
        }
        
        std::thread thrd(&TerminalMap::_on_month_tick_timeout_helper, this, start, end);
        month_threads.push_back(std::move(thrd));
        start = end;
    }
    // _on_month_tick_timeout_helper(cargo_map_terminals.begin(), cargo_map_terminals.end());
}

void TerminalMap::_on_month_tick_timeout_helper(MapType::iterator start, MapType::iterator end) {
    for (auto it = start; it != end; it++) {
        while (true) {
            m.lock();
            bool do_break = !day_tick_priority;
            m.unlock();
            if (do_break) break;
            std::this_thread::sleep_for(std::chrono::milliseconds(2));
        }
        Vector2i coords = it -> first;
        Ref<Terminal> terminal = it -> second;
        
        if (terminal->has_method("month_tick")) {
            terminal->call("month_tick");
        }
    }
}

void TerminalMap::clear() {
    m.lock();
    cargo_map_terminals.clear();
    m.unlock();
}

TileMapLayer* TerminalMap::get_main_map() const {
    return map;
}

//Creators
void TerminalMap::create_terminal(Ref<Terminal> p_terminal) {
    m.lock();
    cargo_map_terminals[p_terminal -> get_location()] = p_terminal;
    m.unlock();
    Ref<Broker> broker = get_broker(p_terminal -> get_location());
    if (broker.is_valid()) {
        add_connected_brokers(broker);
    }
}

void TerminalMap::add_connected_brokers(Ref<Broker> p_broker) {
    Array connected = map->get_surrounding_cells(p_broker->get_location());
    for (int i = 0; i < connected.size(); i++) {
        if (connected[i].get_type() != Variant::VECTOR2I) continue;
        Vector2i tile = connected[i];
        Ref<Broker> other = get_broker(tile);
        if (other.is_null()) continue;
        p_broker->add_connected_broker(other);
        other->add_connected_broker(p_broker);
    }
}

Ref<Factory> TerminalMap::create_factory(const Vector2i &p_location, int p_player_owner, const Dictionary &p_inputs, const Dictionary &p_outputs) {
    if (p_player_owner > 0) {
        Ref<Factory> factory;
        factory.instantiate();
        factory->initialize(p_location, p_player_owner, p_inputs, p_outputs);
        return factory;
    } else {
        Ref<AiFactory> factory;
        factory.instantiate();
        factory->initialize(p_location, p_player_owner, p_inputs, p_outputs);
        return factory;
    }
}

//Checkers
bool TerminalMap::is_terminal(const Vector2i &coords) {
    return cargo_map_terminals.count(coords) == 1;
}

bool TerminalMap::is_hold(const Vector2i &coords) {
    return get_terminal_as<Hold>(coords).is_valid();
}
bool TerminalMap::is_owned_recipeless_construction_site(const Vector2i &coords) {
    bool toReturn = false;
    Ref<ConstructionSite> construction_site =  get_terminal_as<ConstructionSite>(coords);
    if (construction_site.is_null()) return false;
    
    toReturn = construction_site -> has_recipe();

    return toReturn;
}
bool TerminalMap::is_building(const Vector2i &coords) {
    bool toReturn = false;
    if (!cargo_map_terminals.count(coords)) return false;
    toReturn = get_terminal_as<ConstructionSite>(coords).is_valid() ||  get_terminal_as<Town>(coords).is_valid() ||  get_terminal_as<FactoryTemplate>(coords).is_valid();
    return toReturn;
}
bool TerminalMap::is_owned_building(const Vector2i &coords, int id) {
    if (is_building(coords)) {
        return cargo_map_terminals[coords] -> get_player_owner() == id;
    }
    return false;
}
bool TerminalMap::is_owned_construction_site(const Vector2i &coords) {
   return get_terminal_as<ConstructionSite>(coords).is_valid();
}
bool TerminalMap::is_factory(const Vector2i &coords) {
    return get_terminal_as<Factory>(coords).is_valid();
}
bool TerminalMap::is_station(const Vector2i &coords) {
    return get_terminal_as<StationWOMethods>(coords).is_valid();
}
bool TerminalMap::is_road_depot(const Vector2i &coords) {
    return get_terminal_as<RoadDepotWOMethods>(coords).is_valid();
}
bool TerminalMap::is_owned_station(const Vector2i &coords, int player_id) {
    Ref<StationWOMethods> temp = get_terminal_as<StationWOMethods>(coords);
    if (temp.is_valid()) {
        return temp->get_player_owner() == player_id;
    }
    return false;
}
bool TerminalMap::is_ai_station(const Vector2i &coords) {                       //May work?
    bool val = false;
    m.lock();
    Ref<StationWOMethods> term = get_terminal_as<StationWOMethods>(coords);
    if (term.is_valid()) {
        val = term -> get_class() == "AiStation";
    }
    m.unlock();
    return val;
} 
bool TerminalMap::is_owned_ai_station(const Vector2i &coords, int id) {
    bool val = false;
    if (is_ai_station(coords)) {
        Ref<StationWOMethods> station = get_terminal_as<StationWOMethods>(coords);
        val = station -> get_player_owner() == id;
    }
    return val;
} 
bool TerminalMap::is_town(const Vector2i &coords) {
    return get_terminal_as<Town>(coords).is_valid();
}

//Info getters
Dictionary TerminalMap::get_cargo_dict(const Vector2i &coords) {
    Dictionary d;
    Ref<Hold> hold = get_terminal_as<Hold>(coords);
    if (hold.is_null()) return d;

    d = hold -> get_current_hold();
    return d;
}


Array TerminalMap::get_construction_site_recipe(const Vector2i &coords) {
    Array toReturn = {};
    Ref<Terminal> temp = get_terminal(coords);
    Ref<ConstructionSite> construction_site = get_terminal_as<ConstructionSite>(coords);
    if (construction_site.is_valid()) {
        toReturn = construction_site->get_recipe();
    }
    return toReturn;
}

Dictionary TerminalMap::get_construction_materials(const Vector2i &coords) {
    Dictionary toReturn;
    if (is_owned_construction_site(coords)) {
        toReturn = get_terminal_as<ConstructionSite>(coords) -> get_construction_materials();
    }
    return toReturn;
}

int TerminalMap::get_cash_of_firm(const Vector2i &coords) {
    int toReturn;
    Ref<Firm> firm = get_terminal_as<Firm>(coords);
    if (firm.is_valid()) {
        toReturn = firm->get_cash();
    }
    return toReturn;
}

Dictionary TerminalMap::get_local_prices(const Vector2i &coords) {
    Dictionary toReturn;
    Ref<Broker> broker = get_broker(coords);
    if (broker.is_valid()) {
        toReturn = broker -> get_local_prices();
    }
    return toReturn;
}

Dictionary TerminalMap::get_station_orders(const Vector2i &coords) {
    Dictionary toReturn;
    if (is_station(coords)) {
        toReturn = get_terminal_as<StationWOMethods>(coords) -> get_orders_dict();
    }
    return toReturn;
}

Dictionary TerminalMap::get_town_fulfillment(const Vector2i &coords) {
    Dictionary toReturn;
    Ref<Town> town = get_town(coords);
    if (town.is_valid()) {
        toReturn = town -> get_fulfillment_dict();
    }
    return toReturn;
}

//Internal Getters
Ref<Terminal> TerminalMap::get_terminal(const Vector2i &coords) {
    return get_terminal_as<Terminal>(coords);
}
Ref<Broker> TerminalMap::get_broker(const Vector2i &coords) {
    return get_terminal_as<Broker>(coords);
}
Ref<StationWOMethods> TerminalMap::get_station(const Vector2i &coords) {
    return get_terminal_as<StationWOMethods>(coords);
}
Ref<StationWOMethods> TerminalMap::get_ai_station(const Vector2i &coords) {
    return get_terminal_as<StationWOMethods>(coords, [](const Vector2i &pos) { return TerminalMap::get_instance()->is_ai_station(pos);});
}
Ref<Town> TerminalMap::get_town(const Vector2i &coords) {
    return get_terminal_as<Town>(coords);
}

template <typename T>
Ref<T> TerminalMap::get_terminal_as(const Vector2i &coords, const std::function<bool(const Vector2i &)> &type_check) {
    std::scoped_lock lock(m);
    Ref<T> toReturn = Ref<T>(nullptr);
    if (cargo_map_terminals.count(coords) == 1 && (!type_check || type_check(coords))) {
        Ref<T> typed = cargo_map_terminals[coords];
        if (typed.is_valid()) {
            toReturn = typed;
        }
    }
    return toReturn;
}

//Action doers
void TerminalMap::set_construction_site_recipe(const Vector2i &coords, const Array &selected_recipe) {
    if (is_owned_recipeless_construction_site(coords)) {
        get_terminal_as<ConstructionSite>(coords) -> set_recipe(selected_recipe);
    }
}

void TerminalMap::destroy_recipe(const Vector2i &coords) {
    if (is_owned_recipeless_construction_site(coords)) {
        get_terminal_as<ConstructionSite>(coords) -> destroy_recipe();
    }
}

void TerminalMap::transform_construction_site_to_factory(const Vector2i &coords) {
    if (is_owned_construction_site(coords)) {
        
        Ref<ConstructionSite> old_site = get_terminal_as<ConstructionSite>(coords);
        cargo_map_terminals[coords] = create_factory(coords, old_site->get_player_owner(), old_site->get_recipe()[0], old_site->get_recipe()[1]);

    }
}

void TerminalMap::edit_order_station(const Vector2i &coords, int type, int amount, bool buy, float max_price) {
    if (is_station(coords)) {

        Ref<StationWOMethods> station = get_terminal_as<StationWOMethods>(coords);
        station -> edit_order(type, amount, buy, max_price);

    }
}

void TerminalMap::remove_order_station(const Vector2i &coords, int type) {
    if (is_station(coords)) {

        Ref<StationWOMethods> station = get_terminal_as<StationWOMethods>(coords);
        station -> remove_order(type);

    }
}