#include "terminal_map.hpp"
#include "road_map.hpp"
#include "recipe_info.hpp"
#include "province_manager.hpp"
#include "../classes/terminal.hpp"
#include "../classes/station.hpp"
#include "../classes/broker.hpp"
#include "../classes/Factory.hpp"
#include "../classes/ai_factory.hpp"
#include "../classes/town.hpp"
#include "../classes/road_depot.hpp"


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

    ClassDB::bind_method(D_METHOD("create_terminal", "p_terminal"), &TerminalMap::create_terminal);
    ClassDB::bind_method(D_METHOD("encode_factory", "factory", "mult"), &TerminalMap::encode_factory);
    ClassDB::bind_method(D_METHOD("add_connected_brokers", "p_broker"), &TerminalMap::add_connected_brokers);
    ClassDB::bind_method(D_METHOD("create_factory", "p_location", "p_player_owner", "p_inputs", "p_outputs"), &TerminalMap::create_factory);
    ClassDB::bind_method(D_METHOD("create_primary_factory", "p_location", "p_player_owner", "type"), &TerminalMap::create_primary_factory);
    

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
    ClassDB::bind_method(D_METHOD("get_available_primary_recipes", "coords"), &TerminalMap::get_available_primary_recipes);
    

    ClassDB::bind_method(D_METHOD("get_terminal", "coords"), &TerminalMap::get_terminal);
    ClassDB::bind_method(D_METHOD("get_broker", "coords"), &TerminalMap::get_broker);
    ClassDB::bind_method(D_METHOD("get_station", "coords"), &TerminalMap::get_station);
    ClassDB::bind_method(D_METHOD("get_ai_station", "coords"), &TerminalMap::get_ai_station);
    ClassDB::bind_method(D_METHOD("get_town", "coords"), &TerminalMap::get_town);
    ClassDB::bind_method(D_METHOD("get_factory", "coords"), &TerminalMap::get_factory);

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
    singleton_instance->thread_pool = new TerminalMapThreadPool;
    
}

TerminalMap::TerminalMap() {
}

TerminalMap::~TerminalMap() {
    //Clean up old threads
    cargo_map_terminals.clear();
    delete thread_pool;
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
    thread_pool->day_tick();
}

void TerminalMap::_on_month_tick_timeout() {
    thread_pool->month_tick();
}

std::vector<Ref<Terminal>> TerminalMap::get_terminals_for_day_tick() const {
    std::vector<Ref<Terminal>> v;
    {
        std::shared_lock lock(cargo_map_mutex);
        for (const auto &[__, terminal]: terminal_id_to_terminal) {
            if (terminal->has_method("day_tick")) {
                v.push_back(terminal);
            }
        }
    }
    
    return v;
}

std::vector<Ref<Terminal>> TerminalMap::get_terminals_for_month_tick() const {
    std::vector<Ref<Terminal>> v;
    {
        std::shared_lock lock(cargo_map_mutex);
        for (const auto &[__, terminal]: terminal_id_to_terminal) {
            if (terminal->has_method("month_tick")) {
                v.push_back(terminal);
            }
        }
    }
    return v;
}

void TerminalMap::clear() {
    std::unique_lock lock(cargo_map_mutex);
    cargo_map_terminals.clear();
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
void TerminalMap::create_isolated_terminal(Ref<Terminal> p_terminal) {
    int term_id = p_terminal->get_terminal_id();
    {
        std::unique_lock lock(cargo_map_mutex);
        ERR_FAIL_COND_MSG(terminal_id_to_terminal.count(term_id), "Tried to create terminal where terminal exists with id " + String::num_int64(term_id));
        terminal_id_to_terminal[term_id] = p_terminal;
    }
}

void TerminalMap::create_terminal(Ref<Terminal> p_terminal) {
    Vector2i location = p_terminal->get_location();
    int term_id = p_terminal->get_terminal_id();
    {
        std::unique_lock lock(cargo_map_mutex);
        ERR_FAIL_COND_MSG(cargo_map_terminals.count(location) || terminal_id_to_terminal.count(term_id), "Tried to create terminal where terminal exists at " + location + " with id " + String::num_int64(term_id));
        cargo_map_terminals[location] = term_id;
        terminal_id_to_terminal[term_id] = p_terminal;
    }
    Ref<Broker> broker = get_broker(location);
    if (broker.is_valid()) {
        add_connected_brokers(broker);
        find_stations(broker);
    }
    Ref<RoadDepot> road_depot = get_terminal_as<RoadDepot>(location);
    if (road_depot.is_valid()) {
        add_connected_stations(road_depot);
    }
}

void TerminalMap::encode_factory(Ref<Factory> factory, int mult) {
    for (int i = 1; i < mult; i++) {
        factory->admin_upgrade();
    }
    Ref<ProvinceManager> province_manager = ProvinceManager::get_instance();
    Vector2i coords = factory->get_location();
    Province* province = province_manager->get_province(province_manager->get_province_id(coords));
    if (province == nullptr) {
        print_error("Province not found with tile : " + coords);
        return;
    }
    
    create_terminal(factory);
    province->add_terminal(coords); // Adds to province
    cargo_map->call("call_set_tile_rpc", coords, factory->get_primary_type());
}

void TerminalMap::encode_factory_from_construction_site(Ref<Factory> factory) {
    Vector2i coords = factory->get_location();
    create_terminal(factory);
    cargo_map->call("call_set_tile_rpc", coords, factory->get_primary_type());
}

void TerminalMap::add_connected_brokers(Ref<Broker> p_broker) {
    Array connected = map->get_surrounding_cells(p_broker->get_location());
    for (int i = 0; i < connected.size(); i++) {
        if (connected[i].get_type() != Variant::VECTOR2I) continue;
        Vector2i tile = connected[i];
        Ref<Broker> other = get_terminal_as<Broker>(tile);
        if (other.is_null()) continue;
        p_broker->add_connected_broker(other);
        other->add_connected_broker(p_broker);
    }
}

void TerminalMap::add_connected_stations(Ref<RoadDepot> road_depot) {
    Array tiles = map->get_surrounding_cells(road_depot->get_location());
    for (int i = 0; i < tiles.size(); i++) {
        Vector2i tile = tiles[i];
        Ref<Broker> broker = get_broker(tile);
        if (broker.is_valid()) {
            broker->add_connected_station(road_depot->get_location());
            road_depot->add_connected_broker(broker);
        }
    }
}

void TerminalMap::find_stations(Ref<Broker> broker) {
    Array tiles = map->get_surrounding_cells(broker->get_location());
    for (int i = 0; i < tiles.size(); i++) {
        Vector2i tile = tiles[i];
        Ref<RoadDepot> road_depot = get_terminal_as<RoadDepot>(tile);
        if (road_depot.is_valid()) {
            broker->add_connected_station(tile);
            road_depot->add_connected_broker(broker);
        }
    }
}

Ref<Factory> TerminalMap::create_factory(const Vector2i &p_location, int p_player_owner, const Dictionary &p_inputs, const Dictionary &p_outputs) {
    if (p_player_owner > 0) {
        Ref<Factory> factory;
        factory.instantiate();
        
        factory->initialize(p_location, p_player_owner, RecipeInfo::get_instance()->get_recipe(p_inputs, p_outputs));
        return factory;
    } else {
        Ref<AiFactory> factory;
        factory.instantiate();
        factory->initialize(p_location, p_player_owner, RecipeInfo::get_instance()->get_recipe(p_inputs, p_outputs));
        return factory;
    }
}

Ref<Factory> TerminalMap::create_primary_factory(const Vector2i &p_location, int p_player_owner, int type) const {
    if (p_player_owner > 0) {
        Ref<Factory> factory = Ref<Factory>(memnew(Factory(p_location, p_player_owner, RecipeInfo::get_instance()->get_primary_recipe_for_type(type))));
        return factory;
    } else {
        Ref<AiFactory> factory = Ref<AiFactory>(memnew(AiFactory(p_location, p_player_owner, RecipeInfo::get_instance()->get_primary_recipe_for_type(type))));
        return factory;
    }
}

//Checkers
int TerminalMap::get_cargo_value_of_tile(const Vector2i &coords, int type) const {
    std::scoped_lock lock(m);
    return int(cargo_values->call("get_tile_magnitude", coords, type)) * 10;
}

std::vector<int> TerminalMap::get_available_resources_of_tile(const Vector2i &coords) const {
    std::vector<int> toReturn;
    for (int type = 0; type < CargoInfo::get_instance()->get_amount_of_primary_goods(); type++) {
        toReturn.push_back(get_cargo_value_of_tile(coords, type));
    }
    return toReturn;
}

bool TerminalMap::is_terminal(const Vector2i &coords) {
    std::shared_lock lock(cargo_map_mutex);
    return cargo_map_terminals.count(coords) == 1;
}

bool TerminalMap::is_hold(const Vector2i &coords) {
    return get_terminal_as<Hold>(coords).is_valid();
}
bool TerminalMap::is_owned_recipeless_construction_site(const Vector2i &coords) {
    Ref<ConstructionSite> construction_site =  get_terminal_as<ConstructionSite>(coords);
    if (construction_site.is_null()) return false;
    
    bool toReturn = !(construction_site -> has_recipe());

    return toReturn;
}
bool TerminalMap::is_building(const Vector2i &coords) {
    bool toReturn = false;
    {
        std::shared_lock lock(cargo_map_mutex);
        if (!cargo_map_terminals.count(coords)) return false;
    }
    toReturn = get_terminal_as<ConstructionSite>(coords).is_valid() ||  get_terminal_as<Town>(coords).is_valid() ||  get_terminal_as<FactoryTemplate>(coords).is_valid();
    return toReturn;
}
bool TerminalMap::is_owned_building(const Vector2i &coords, int id) {
    if (is_building(coords)) {
        return get_terminal(coords) -> get_player_owner() == id;
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
    return get_terminal_as<RoadDepot>(coords).is_valid();
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
    std::unique_lock lock(cargo_map_mutex);
    Ref<StationWOMethods> term = get_terminal_as<StationWOMethods>(coords);
    if (term.is_valid()) {
        val = term -> get_class() == "AiStation";
    }
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
    Array toReturn;
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

bool TerminalMap::is_tile_traversable(const Vector2i& coords, bool is_water_untraversable) {
    std::scoped_lock lock(m);
    Vector2i atlas = map -> get_cell_atlas_coords(coords);
    std::unordered_set<Vector2i, godot_helpers::Vector2iHasher> s = {Vector2i(-1, -1), Vector2i(5, 0), Vector2i(7, 0), Vector2i(3, 3)};
    if (is_water_untraversable) s.insert(Vector2i(6, 0));

    return (!s.count(atlas));
}

bool TerminalMap::is_tile_available(const Vector2i& coords) {
    bool status;
    {
        std::shared_lock lock(cargo_map_mutex);
        status = !cargo_map_terminals.count(coords);
    }
    return status && is_tile_traversable(coords, true);
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

Ref<Factory> TerminalMap::get_factory(const Vector2i &coords) {
    return get_terminal_as<Factory>(coords);
}

//Action doers
void TerminalMap::set_construction_site_recipe(const Vector2i &coords, const Array &selected_recipe) {
    if (is_owned_recipeless_construction_site(coords)) {
        get_terminal_as<ConstructionSite>(coords) -> set_recipe(RecipeInfo::get_instance()->get_recipe(selected_recipe[0], selected_recipe[1]));
    }
}

void TerminalMap::destroy_recipe(const Vector2i &coords) {
    if (is_owned_recipeless_construction_site(coords)) {
        get_terminal_as<ConstructionSite>(coords) -> destroy_recipe();
    }
}

void TerminalMap::transform_construction_site_to_factory(const Vector2i &coords) { // Doesn't keep same id
    if (is_owned_construction_site(coords)) {
        Ref<ConstructionSite> old_site = get_terminal_as<ConstructionSite>(coords);
        Ref<Factory> factory = create_factory(coords, old_site->get_player_owner(), old_site->get_recipe()[0], old_site->get_recipe()[1]);

        {
            std::unique_lock lock(cargo_map_mutex);
            terminal_id_to_terminal.erase(old_site->get_terminal_id());
            cargo_map_terminals.erase(old_site->get_location());
        }
        

        encode_factory_from_construction_site(factory);

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

void TerminalMap::refresh_road_depots(const std::unordered_set<Vector2i, godot_helpers::Vector2iHasher> &s) {
    for (auto it = s.begin(); it != s.end(); it++) {
        Ref<RoadDepot> road_depot = get_terminal_as<RoadDepot>(*it);
        if (road_depot.is_valid()) road_depot -> refresh_other_road_depots();
    }
}


float TerminalMap::get_average_cash_of_road_depot() const {
    return get_average_cash_of_terminal<RoadDepot>();
}

float TerminalMap::get_average_cash_of_factory() const {
    return get_average_cash_of_terminal<Factory>();
}

float TerminalMap::get_average_factory_level() const {
    double ave = 0;
    int count = 0;
    std::shared_lock lock(cargo_map_mutex);
    for (const auto &[__, terminal]: terminal_id_to_terminal) {
        Ref<Factory> typed = Ref<Factory>(terminal);
        if (typed.is_valid()) {
            ave += typed -> get_level();
            count++;
        }
    }
    return ave / count;
}

int TerminalMap::get_grain_demand() const {
    double total_demand = 0;
    std::shared_lock lock(cargo_map_mutex);
    for (const auto &[__, terminal]: terminal_id_to_terminal) {
        Ref<Town> typed = Ref<Town>(terminal);
        if (typed.is_valid()) {
            double num = double(typed -> get_last_month_demand().get(CargoInfo::get_instance()->get_cargo_type("grain"), 0.0));
            total_demand += num;
        }
    }
    return round(total_demand);
}


int TerminalMap::get_grain_supply() const {
    float total_supply = 0;
    std::shared_lock lock(cargo_map_mutex);
    for (const auto &[__, terminal]: terminal_id_to_terminal) {
        Ref<Town> typed = Ref<Town>(terminal);
        if (typed.is_valid()) {
            total_supply += float(typed -> get_last_month_supply().get(CargoInfo::get_instance()->get_cargo_type("grain"), 0.0f));
        }
    }
    return round(total_supply);
}

template <typename T>
float TerminalMap::get_average_cash_of_terminal() const {
    double ave = 0;
    int count = 0;
    std::shared_lock lock(cargo_map_mutex);
    for (const auto &[__, terminal]: terminal_id_to_terminal) {
        Ref<T> typed = Ref<T>(terminal);
        if (typed.is_valid()) {
            ave += typed -> get_cash();
            count++;
        }
    }
    return ave / count;
}