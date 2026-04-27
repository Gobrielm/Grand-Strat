#include "terminal_map.hpp"
#include "road_map.hpp"
#include "recipe_info.hpp"
#include "province_manager.hpp"
#include "cargo_info.hpp"

#include "classes/map_objects/station.hpp"
#include "trading_system.hpp"


using namespace godot;

Ref<TerminalMap> TerminalMap::singleton_instance = nullptr;

void TerminalMap::_bind_methods() {

    ClassDB::bind_static_method(get_class_static(), D_METHOD("is_instance_created"), &TerminalMap::is_instance_created);
    ClassDB::bind_static_method(get_class_static(), D_METHOD("get_instance"), &TerminalMap::get_instance);
    ClassDB::bind_static_method(get_class_static(), D_METHOD("create", "p_map"), &TerminalMap::initialize_singleton);
    // Initialization
    ClassDB::bind_method(D_METHOD("assign_cargo_map", "p_cargo_map"), &TerminalMap::assign_cargo_map);
    ClassDB::bind_method(D_METHOD("assign_cargo_controller", "p_cargo_controller"), &TerminalMap::assign_cargo_controller);

    // Process hooks
    ClassDB::bind_method(D_METHOD("_on_day_tick_timeout"), &TerminalMap::_on_day_tick_timeout);
    ClassDB::bind_method(D_METHOD("_on_month_tick_timeout"), &TerminalMap::_on_month_tick_timeout);

    // Core functions
    ClassDB::bind_method(D_METHOD("clear"), &TerminalMap::clear);
    ClassDB::bind_method(D_METHOD("get_main_map"), &TerminalMap::get_main_map);

    // Creators

    // ClassDB::bind_method(D_METHOD("add_connected_brokers", "p_broker"), &TerminalMap::add_connected_brokers);
    // ClassDB::bind_method(D_METHOD("create_factory", "p_location", "p_player_owner", "p_inputs", "p_outputs"), &TerminalMap::create_factory);
    // ClassDB::bind_method(D_METHOD("create_primary_factory", "p_location", "p_player_owner", "type"), &TerminalMap::create_primary_factory);

    // // Info getters

    ClassDB::bind_method(D_METHOD("get_cargo_dict", "coords"), &TerminalMap::get_cargo_dict);
    // ClassDB::bind_method(D_METHOD("get_construction_site_recipe", "coords"), &TerminalMap::get_construction_site_recipe);
    // ClassDB::bind_method(D_METHOD("get_construction_materials", "coords"), &TerminalMap::get_construction_materials);
    // ClassDB::bind_method(D_METHOD("get_needed_construction_materials", "coords"), &TerminalMap::get_needed_construction_materials);
    // ClassDB::bind_method(D_METHOD("get_cash_of_firm", "coords"), &TerminalMap::get_cash_of_firm);
    // ClassDB::bind_method(D_METHOD("get_local_prices", "coords"), &TerminalMap::get_local_prices);
    // ClassDB::bind_method(D_METHOD("get_station_orders", "coords"), &TerminalMap::get_station_orders);
    ClassDB::bind_method(D_METHOD("get_available_primary_recipes", "coords"), &TerminalMap::get_available_primary_recipes);

    // Action doers
    // ClassDB::bind_method(D_METHOD("set_construction_site_recipe", "coords", "selected_recipe"), &TerminalMap::set_construction_site_recipe_godot);
    // ClassDB::bind_method(D_METHOD("destroy_recipe", "coords"), &TerminalMap::destroy_recipe);
    // ClassDB::bind_method(D_METHOD("transform_construction_site_to_factory", "coords"), &TerminalMap::transform_construction_site_to_factory);
    // ClassDB::bind_method(D_METHOD("edit_order_station", "coords", "type", "amount", "buy", "max_price"), &TerminalMap::edit_order_station);
    // ClassDB::bind_method(D_METHOD("remove_order_station", "coords", "type"), &TerminalMap::remove_order_station);
}

void TerminalMap::initialize_singleton(TileMapLayer* p_map) {
    ERR_FAIL_COND_MSG(singleton_instance != nullptr, "Cannot create multiple instances of singleton!");
    singleton_instance.instantiate();
    singleton_instance -> map = p_map;
    // singleton_instance->thread_pool = new TerminalMapThreadPool;
    
}

TerminalMap::TerminalMap() {
}

TerminalMap::~TerminalMap() {
    //Clean up old threads
    // delete thread_pool;
}

bool TerminalMap::is_instance_created() {
    return singleton_instance != nullptr;
}

Ref<TerminalMap> TerminalMap::get_instance() { 
    ERR_FAIL_COND_V_MSG(singleton_instance == nullptr, nullptr, "Terminal Map has not been created yet");
    return singleton_instance;
}

void TerminalMap::assign_cargo_map(TileMapLayer* p_cargo_map) {
    cargo_map = p_cargo_map;
    cargo_values = cargo_map->get_node<Node2D>("cargo_values");
}

void TerminalMap::assign_cargo_controller(Node* p_cargo_controller) {
    cargo_controller = p_cargo_controller;
}

void TerminalMap::_on_day_tick_timeout() {
    // thread_pool->day_tick();
}

void TerminalMap::_on_month_tick_timeout() {
    // PopManager::get_instance()->month_tick();
    // AiManager::get_instance()->month_tick();
    // thread_pool->month_tick();
    TradingSystem::get_instance()->month_tick();
}

// std::vector<Ref<Terminal>> TerminalMap::get_terminals_for_day_tick() const {
//     std::vector<Ref<Terminal>> v;
//     {
//         std::shared_lock lock(cargo_map_mutex);
//         for (const auto &[__, terminal]: terminal_id_to_terminal) {
//             if (terminal->has_method("day_tick")) {
//                 v.push_back(terminal);
//             }
//         }
//     }
    
//     return v;
// }

// std::vector<Ref<Terminal>> TerminalMap::get_terminals_for_month_tick() const {
//     std::vector<Ref<Terminal>> v;
//     {
//         std::shared_lock lock(cargo_map_mutex);
//         for (const auto &[__, terminal]: terminal_id_to_terminal) {
//             if (terminal->has_method("month_tick")) {
//                 v.push_back(terminal);
//             }
//         }
//     }
//     return v;
// }

void TerminalMap::clear() {
    std::unique_lock lock(cargo_map_mutex);
    id_to_position_component.clear();
}

TileMapLayer* TerminalMap::get_main_map() const {
    return map;
}

TileMapLayer* TerminalMap::get_cargo_map() const {
    return cargo_map;
}

 //Time
void TerminalMap::pause_time() {
    std::scoped_lock lock(m);
    cargo_controller->call("backend_pause");
}

void TerminalMap::unpause_time() {
    std::scoped_lock lock(m);
    cargo_controller->call("backend_unpause");
}

//Creators
// void TerminalMap::create_isolated_terminal(Ref<Terminal> p_terminal) {
//     Vector2i tile = p_terminal->get_location();
//     Ref<FactoryTemplate> factory = p_terminal;
//     if (is_town(tile) && factory.is_valid()) {
//         create_isolated_factory_in_town(factory);
//         return;
//     }

//     int term_id = p_terminal->get_terminal_id();
//     {
//         std::unique_lock lock(cargo_map_mutex);
//         ERR_FAIL_COND_MSG(terminal_id_to_terminal.count(term_id), "Tried to create terminal where terminal exists with id " + String::num_int64(term_id));
//         terminal_id_to_terminal[term_id] = p_terminal;
//     }
// }
// //TODO
// void TerminalMap::create_isolated_factory_in_town(Factory p_factory) {
//     int term_id = p_factory->get_terminal_id();
//     {
//         std::unique_lock lock(cargo_map_mutex);
//         ERR_FAIL_COND_MSG(terminal_id_to_terminal.count(term_id), "Tried to create terminal where terminal exists with id " + String::num_int64(term_id));
//         terminal_id_to_terminal[term_id] = p_factory;
//     }
//     Vector2i tile = p_factory->get_location();
//     auto pm = ProvinceManager::get_instance();
//     Town& town = pm->get_province(tile)->get_town();
//     town.add_factory(p_factory);
// }

// void TerminalMap::create_isolated_company_in_town(Ref<CompanyAi> p_company) {
//     int term_id = p_company->get_terminal_id();
//     {
//         std::unique_lock lock(cargo_map_mutex);
//         ERR_FAIL_COND_MSG(terminal_id_to_terminal.count(term_id), "Tried to create terminal where terminal exists with id " + String::num_int64(term_id));
//         terminal_id_to_terminal[term_id] = p_company;
//     }
//     Vector2i tile = p_company->get_location();
//     Ref<Town> town = get_town(tile);
//     ERR_FAIL_COND_MSG(town.is_null(), "Adding isolated terminal to invalid town.");
//     town->add_company(p_company);
// }

// void TerminalMap::create_terminal(Ref<Terminal> p_terminal) {
//     Vector2i location = p_terminal->get_location();
//     int term_id = p_terminal->get_terminal_id();
//     {
//         std::unique_lock lock(cargo_map_mutex);
//         ERR_FAIL_COND_MSG(cargo_map_terminals.count(location) || terminal_id_to_terminal.count(term_id), "Tried to create terminal where terminal exists at " + location + " with id " + String::num_int64(term_id));
//         cargo_map_terminals[location] = term_id;
//         terminal_id_to_terminal[term_id] = p_terminal;
//     }
//     Ref<Broker> broker = get_broker(location);
//     if (broker.is_valid()) {
//         add_connected_brokers(broker);
//         find_stations(broker);
//     }
//     Ref<RoadDepot> road_depot = get_terminal_as<RoadDepot>(location);
//     if (road_depot.is_valid()) {
//         add_connected_stations(road_depot);
//     }
// }

void TerminalMap::encode_factory(Factory& factory, int mult) {
    for (int i = 1; i < mult; i++) {
        factory.admin_upgrade();
    }
    Ref<ProvinceManager> province_manager = ProvinceManager::get_instance();
    Vector2i coords = factory.position.get_position_vector2i();
    Province* province = province_manager->get_province(province_manager->get_province_id(coords));
    if (province == nullptr) {
        print_error("Province not found with tile : " + coords);
        return;
    }
    {
        std::scoped_lock lock(m);
        id_to_position_component[factory.position.get_building_id()] = factory.position;
    }
    
    province->add_factory(factory); // Adds to province
    cargo_map->call_deferred("call_set_tile_rpc", coords, factory.get_primary_type()); // Will result in lots of memory when used extensively
}

void TerminalMap::encode_factory_no_calls_to_cargo_map(Factory& factory, int mult) {
    for (int i = 1; i < mult; i++) {
        factory.admin_upgrade();
    }
    Ref<ProvinceManager> province_manager = ProvinceManager::get_instance();
    Vector2i coords = factory.position.get_position_vector2i();
    Province* province = province_manager->get_province(coords);
    if (province == nullptr) {
        print_error("Province not found with tile : " + coords);
        return;
    }
    
    {
        std::scoped_lock lock(m);
        id_to_position_component[factory.position.get_building_id()] = factory.position;
    }

    province->add_factory(factory); // Adds to province
}

void TerminalMap::encode_factory_from_construction_site(Factory& factory) {
    Vector2i coords = factory.position.get_position_vector2i();
    {
        std::scoped_lock lock(m);
        id_to_position_component[factory.position.get_building_id()] = factory.position;
    }
    cargo_map->call("call_set_tile_rpc", coords, factory.get_primary_type());
}

// void TerminalMap::encode_road_depot(Ref<RoadDepot> road_depot) {
//     Ref<ProvinceManager> province_manager = ProvinceManager::get_instance();
//     Vector2i coords = road_depot->get_location();
//     Province* province = province_manager->get_province(province_manager->get_province_id(coords));
    
//     create_terminal(road_depot);
//     province->add_terminal(coords);
//     RoadMap::get_instance()->place_road_depot(coords);
// }

// void TerminalMap::encode_construction_site(Ref<ConstructionSite> construction_site) {
//     Ref<ProvinceManager> province_manager = ProvinceManager::get_instance();
//     Vector2i coords = construction_site->get_location();
//     Province* province = province_manager->get_province(province_manager->get_province_id(coords));
    
//     create_terminal(construction_site);
//     province->add_terminal(coords);
//     cargo_map->call_deferred("place_construction_site_tile", coords);
// }

// void TerminalMap::add_connected_brokers(Ref<Broker> p_broker) {
//     Array connected = map->get_surrounding_cells(p_broker->get_location());
//     for (int i = 0; i < connected.size(); i++) {
//         if (connected[i].get_type() != Variant::VECTOR2I) continue;
//         Vector2i tile = connected[i];
//         Ref<Broker> other = get_terminal_as<Broker>(tile);
//         if (other.is_null()) continue;
//         p_broker->add_connected_broker(other);
//         other->add_connected_broker(p_broker);
//     }
// }

// void TerminalMap::add_connected_stations(Ref<RoadDepot> road_depot) {
//     Array tiles = map->get_surrounding_cells(road_depot->get_location());
//     for (int i = 0; i < tiles.size(); i++) {
//         Vector2i tile = tiles[i];
//         Ref<Broker> broker = get_broker(tile);
//         if (broker.is_valid()) {
//             broker->add_connected_station(road_depot->get_location());
//             road_depot->add_connected_broker(broker);
//         }
//     }
// }

// void TerminalMap::find_stations(Ref<Broker> broker) {
//     Array tiles = map->get_surrounding_cells(broker->get_location());
//     for (int i = 0; i < tiles.size(); i++) {
//         Vector2i tile = tiles[i];
//         Ref<RoadDepot> road_depot = get_terminal_as<RoadDepot>(tile);
//         if (road_depot.is_valid()) {
//             broker->add_connected_station(tile);
//             road_depot->add_connected_broker(broker);
//         }
//     }
// }

// Factory TerminalMap::create_factory(const Vector2i &p_location, int p_player_owner, const Dictionary &p_inputs, const Dictionary &p_outputs) {
//     if (p_player_owner > 0) {
//         Factory factory(std::make_pair(p_location.x, p_location.y), p_player_owner, RecipeInfo::get_instance()->get_recipe(p_inputs, p_outputs).value());

//         return factory;
//     } else {
//         Ref<AiFactory> factory;
//         factory.instantiate();
//         factory->initialize(p_location, p_player_owner, RecipeInfo::get_instance()->get_recipe(p_inputs, p_outputs));
//         return factory;
//     }
// }

// Factory TerminalMap::create_primary_factory(const Vector2i &p_location, int p_player_owner, int type) const {
//     if (p_player_owner > 0) {
//         Factory factory = Factory(std::make_pair(p_location.x, p_location.y), p_player_owner, RecipeInfo::get_instance()->get_primary_recipe_for_type(type).value());
//         return factory;
//     } else {
//         Ref<AiFactory> factory = Ref<AiFactory>(memnew(AiFactory(p_location, p_player_owner, RecipeInfo::get_instance()->get_primary_recipe_for_type(type))));
//         return factory;
//     }
// }

//Checkers
int TerminalMap::get_cargo_value_of_tile(const Vector2i coords, int type) const {
    return int(cargo_values->call("get_tile_magnitude", coords, type)) * 10;
}

std::vector<int> TerminalMap::get_available_resources_of_tile(const Vector2i coords) const {
    std::vector<int> toReturn;
    for (int type = 0; type < CargoInfo::get_instance()->get_amount_of_primary_goods(); type++) {
        toReturn.push_back(get_cargo_value_of_tile(coords, type));
    }
    return toReturn;
}

//Info getters

// Array TerminalMap::get_construction_site_recipe(const Vector2i &coords) {
//     Array toReturn;
//     Ref<ConstructionSite> construction_site = get_terminal_as<ConstructionSite>(coords);
//     if (construction_site.is_valid()) {
//         toReturn = construction_site->get_recipe();
//     }
//     return toReturn;
// }

// Dictionary TerminalMap::get_construction_materials(const Vector2i &coords) {
//     Dictionary toReturn;
//     if (is_owned_construction_site(coords)) {
//         toReturn = get_terminal_as<ConstructionSite>(coords) -> get_construction_materials();
//     }
//     return toReturn;
// }

// Dictionary TerminalMap::get_needed_construction_materials(const Vector2i &coords) {
//     Dictionary toReturn;
//     if (is_owned_construction_site(coords)) {
//         toReturn = get_terminal_as<ConstructionSite>(coords) -> get_needed_construction_materials();
//     }
//     return toReturn;
// }

Dictionary TerminalMap::get_cargo_dict(const Vector2i &coords) {
    Dictionary d;
    auto pm = ProvinceManager::get_instance();
    if (!pm->is_factory(coords)) return d;
    auto province = pm->get_province(coords);
    if (province == nullptr) return d;
    

    d = province->get_factory(province->get_visible_position_component(coords).get_building_id()).storage.dictionary();
    return d;
}


// int TerminalMap::get_cash_of_firm(const Vector2i coords) {
//     int toReturn;
//     Ref<Firm> firm = get_terminal_as<Firm>(coords);
//     if (firm.is_valid()) {
//         toReturn = firm->get_cash();
//     }
//     return toReturn;
// }

// Dictionary TerminalMap::get_local_prices(const Vector2i &coords) {
//     Dictionary toReturn;
//     Ref<Broker> broker = get_broker(coords);
//     if (broker.is_valid()) {
//         toReturn = broker -> get_local_prices();
//     }
//     return toReturn;
// }

// Dictionary TerminalMap::get_station_orders(const Vector2i &coords) {
//     Dictionary toReturn;
//     if (is_station(coords)) {
//         toReturn = get_terminal_as<StationWOMethods>(coords) -> get_orders_dict();
//     }
//     return toReturn;
// }

bool TerminalMap::is_tile_traversable(const Vector2i coords, bool is_water_untraversable) {
    std::scoped_lock lock(m);
    Vector2i atlas = map -> get_cell_atlas_coords(coords);
    std::unordered_set<Vector2i, godot_helpers::Vector2iHasher> s = {Vector2i(-1, -1), Vector2i(5, 0), Vector2i(7, 0), Vector2i(3, 3)};
    if (is_water_untraversable) s.insert(Vector2i(6, 0));

    return (!s.count(atlas));
}

Array TerminalMap::get_available_primary_recipes(const Vector2i& coords) const {
    std::vector<int> v = get_available_resources_of_tile(coords);
    Array a;
    for (int type = 0; type < v.size(); type++) {
        if (v[type] != 0) { 
            Dictionary d;
            d[type] = 1;
            Array a2;
            a2.push_back(Dictionary());
            a2.push_back(d);
            a.push_back(a2);
        }
    }
    return a;
}

void TerminalMap::encode_building(PositionComponent pos) {
    std::scoped_lock lock(m);
    id_to_position_component[pos.get_building_id()] = pos;
}

void TerminalMap::place_object_on_map(PositionComponent pos) {
    auto province = ProvinceManager::get_instance()->get_province(pos.get_position_vector2i());
    switch (pos.get_type()) {
        case BuildingType::FACTORY:
            cargo_map->call_deferred("call_set_tile_rpc", pos.get_position_vector2i(), province->get_factory(pos.get_building_id()).get_primary_type());
        // case BuildingType::TOWN:

        // case BuildingType::STATION:
        
        
    }
}

PositionComponent TerminalMap::get_position_component(int pos_id) const {
    if (!id_to_position_component.count(pos_id)) {
        ERR_FAIL_V_MSG(PositionComponent(), "Tried to access invalid position id: " + String(std::to_string(pos_id).c_str()));
    }
    return id_to_position_component.at(pos_id);
}

//Action doers
// void TerminalMap::set_construction_site_recipe(const Vector2i &coords, Recipe* selected_recipe) {
//     if (is_owned_recipeless_construction_site(coords)) {
//         get_terminal_as<ConstructionSite>(coords) -> set_recipe(selected_recipe);
//     }
// }

// void TerminalMap::set_construction_site_recipe_godot(const Vector2i &coords, const Array &selected_recipe) {
//     if (is_owned_recipeless_construction_site(coords)) {
//         get_terminal_as<ConstructionSite>(coords) -> set_recipe(RecipeInfo::get_instance()->get_recipe(selected_recipe[0], selected_recipe[1]).value());
//     }
// }

// void TerminalMap::destroy_recipe(const Vector2i &coords) {
//     if (is_owned_recipeless_construction_site(coords)) {
//         get_terminal_as<ConstructionSite>(coords) -> destroy_recipe();
//     }
// }

// void TerminalMap::transform_construction_site_to_factory(const Vector2i &coords) { // Doesn't keep same id
//     if (is_owned_construction_site(coords)) {
//         Ref<ConstructionSite> old_site = get_terminal_as<ConstructionSite>(coords);
//         Factory factory = create_factory(coords, old_site->get_player_owner(), old_site->get_recipe()[0], old_site->get_recipe()[1]);
//         {
//             std::unique_lock lock(cargo_map_mutex);
//             terminal_id_to_terminal.erase(old_site->get_terminal_id());
//             cargo_map_terminals.erase(old_site->get_location());
//         }
//         encode_factory_from_construction_site(factory);

//     }
// }

// void TerminalMap::edit_order_station(const Vector2i &coords, int type, int amount, bool buy, float max_price) {
//     if (is_station(coords)) {

//         Ref<StationWOMethods> station = get_terminal_as<StationWOMethods>(coords);
//         station -> edit_order(type, amount, buy, max_price);

//     }
// }

// void TerminalMap::remove_order_station(const Vector2i &coords, int type) {
//     if (is_station(coords)) {

//         Ref<StationWOMethods> station = get_terminal_as<StationWOMethods>(coords);
//         station -> remove_order(type);
//     }
// }

// void TerminalMap::refresh_road_depots(const std::unordered_set<Vector2i, godot_helpers::Vector2iHasher> &s) {
//     for (auto it = s.begin(); it != s.end(); it++) {
//         Ref<RoadDepot> road_depot = get_terminal_as<RoadDepot>(*it);
//         if (road_depot.is_valid()) road_depot -> refresh_other_road_depots();
//     }
// }