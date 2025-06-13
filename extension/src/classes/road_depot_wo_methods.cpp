#include "road_depot_wo_methods.hpp"
#include "../singletons/terminal_map.hpp"

//TODO: Problems with road depots selling in between each other, and depots selling to towns and towns selling right back
//Build a custom inherited local pricer that uses supply / demand to have its own local price, then just sell profitably, and will go in the right direction
//Then 'island' depots can still trade and have local market, good for supply army method.

void RoadDepotWOMethods::_bind_methods() {
    ClassDB::bind_static_method(get_class_static(), D_METHOD("create", "new_location", "player_owner"), &RoadDepotWOMethods::create);
    ClassDB::bind_method(D_METHOD("initialize", "new_location", "player_owner"), &RoadDepotWOMethods::initialize);

    // Expose methods to GDScript
    ClassDB::bind_method(D_METHOD("add_connected_road_depot", "road_depot"), &RoadDepotWOMethods::add_connected_road_depot);
    ClassDB::bind_method(D_METHOD("remove_connected_road_depot", "road_depot"), &RoadDepotWOMethods::remove_connected_road_depot);

    ClassDB::bind_method(D_METHOD("month_tick"), &RoadDepotWOMethods::month_tick);

    GDVIRTUAL_BIND(supply_armies);
}

Ref<RoadDepotWOMethods> RoadDepotWOMethods::create(const Vector2i new_location, const int player_owner) {
    Ref<RoadDepotWOMethods> toReturn = Ref<RoadDepotWOMethods>(memnew (RoadDepotWOMethods(new_location, player_owner)));
    return toReturn;
}

RoadDepotWOMethods::RoadDepotWOMethods(): StationWOMethods() {}
RoadDepotWOMethods::RoadDepotWOMethods(Vector2i new_location, int player_owner): StationWOMethods(new_location, player_owner) {
    local_pricer = memnew(LocalPriceController);
    search_for_and_add_road_depots(); //Constructor runs this before being added to TerminalMap
}

RoadDepotWOMethods::~RoadDepotWOMethods() {
    Ref<TerminalMap> terminal_map = TerminalMap::get_instance();
    for (const auto &tile: other_road_depots) {
        Ref<RoadDepotWOMethods> road_depot = terminal_map -> get_terminal_as<RoadDepotWOMethods>(tile);
        if (road_depot.is_null()) continue;

        // terminal_map -> lock(tile);
        road_depot -> remove_connected_road_depot(this);
        // terminal_map -> unlock(tile);
    }
}

//Called by godot
void RoadDepotWOMethods::initialize(Vector2i new_location, int player_owner) {
    local_pricer = memnew(LocalPriceController);
    StationWOMethods::initialize(new_location, player_owner);
    search_for_and_add_road_depots();
    
}

void RoadDepotWOMethods::distribute_cargo() {
    cargo_sent = 0;
    for (const auto &[type, __]: storage) {
        distribute_type(type); //Continue even if amount is 0 because demand needs to be checked
    }	
}

void RoadDepotWOMethods::distribute_type(int type) {
    Ref<TerminalMap> terminal_map = TerminalMap::get_instance();
    //Prioritize local before sending onward
    for (const auto &tile: connected_brokers) {
        Ref<Broker> broker = terminal_map->get_broker(tile);
        if (broker.is_null()) continue;
        bool val = broker -> does_accept(type) && broker -> get_player_owner() == get_player_owner();
        if (val) {
			distribute_type_to_broker(type, broker);
        }
    }
    for (const auto &tile: other_road_depots) {
        //Must be owned by same person
        Ref<RoadDepotWOMethods> road_depot = terminal_map -> get_terminal_as<RoadDepotWOMethods>(tile);
        if (road_depot.is_null()) continue;
        bool val = road_depot -> does_accept(type) && road_depot -> get_player_owner() == get_player_owner();

		if (val) {
            distribute_type_to_road_depot(type, road_depot);
        }
    }
}

void RoadDepotWOMethods::distribute_type_to_broker(int type, Ref<Broker> broker) {
    Ref<TerminalMap> terminal_map = TerminalMap::get_instance();
    Vector2i tile = broker -> get_location();

    float price1 = get_local_price(type);
    float price2 = broker->get_local_price(type);
    
    float price = std::max(price1, price2) - std::abs(price1 - price2) / 2.0f;

    if (!is_price_acceptable_to_sell(type, price) || !broker->is_price_acceptable(type, price)) return;
    int amount_desired = broker -> get_desired_cargo_from_train(type);

    m.lock();
    local_pricer -> add_demand(type, amount_desired);
    m.unlock();

    int amount = std::min(amount_desired, get_cargo_amount(type));
    if (amount != 0) {
        broker -> buy_cargo(type, sell_cargo(type, amount, price), price);
    }
}

void RoadDepotWOMethods::distribute_type_to_road_depot(int type, Ref<RoadDepotWOMethods> road_depot) {
    Ref<TerminalMap> terminal_map = TerminalMap::get_instance();
    Vector2i tile = road_depot -> get_location();

    float price1 = get_local_price(type);

    float price2 = road_depot->get_local_price(type);

    float price = std::max(price1, price2) - std::abs(price1 - price2) / 2.0f;

    
    if (!is_price_acceptable_to_sell(type, price) || !road_depot->is_price_acceptable_to_buy(type, price)) return;
    int amount_desired = road_depot -> get_desired_cargo_from_train(type);

    m.lock();
    local_pricer -> add_demand(type, std::min(amount_desired, MAX_THROUGHPUT));
    m.unlock();
    int amount = std::min(amount_desired, std::min(get_cargo_amount(type), MAX_THROUGHPUT - cargo_sent));
    cargo_sent += amount;

    if (amount != 0) {
        
        road_depot -> buy_cargo(type, sell_cargo(type, amount, price), price);
    }
}

void RoadDepotWOMethods::add_connected_broker(Ref<Broker> broker) {
    Ref<TerminalMap> terminal_map = TerminalMap::get_instance();
    StationWOMethods::add_connected_broker(broker);
    for (const auto &tile: other_road_depots) {
        Ref<RoadDepotWOMethods> road_depot = terminal_map -> get_terminal_as<RoadDepotWOMethods>(tile);
        if (road_depot.is_null()) continue;
        
        road_depot -> add_accepts_from_depot(this);
    }
}

void RoadDepotWOMethods::remove_connected_broker(const Ref<Broker> broker) {
    Ref<TerminalMap> terminal_map = TerminalMap::get_instance();
    StationWOMethods::remove_connected_broker(broker);
    for (const auto &tile: other_road_depots) {
        Ref<RoadDepotWOMethods> road_depot = terminal_map -> get_terminal_as<RoadDepotWOMethods>(tile);
        if (road_depot.is_null()) continue;

        road_depot -> refresh_accepts();
    }
}

void RoadDepotWOMethods::add_connected_road_depot(RoadDepotWOMethods* new_road_depot) {
    // ERR_FAIL_COND_MSG(other_road_depots.count(new_road_depot -> get_location()) != 0, "Already has a road depot there");
    if (other_road_depots.count(new_road_depot -> get_location()) != 0) return; 
    m.lock();
    other_road_depots.insert(new_road_depot -> get_location());
    m.unlock();
    add_accepts_from_depot(new_road_depot);
}

void RoadDepotWOMethods::remove_connected_road_depot(const Ref<RoadDepotWOMethods> new_road_depot) {
    // ERR_FAIL_COND_MSG(other_road_depots.count(new_road_depot -> get_location()) == 0, "Dones't has a road depot there");
    if (other_road_depots.count(new_road_depot -> get_location()) == 0) return; 
    m.lock();
    other_road_depots.erase(new_road_depot -> get_location());
    m.unlock();
    refresh_accepts();
}

void RoadDepotWOMethods::add_accepts_from_depot(const Ref<RoadDepotWOMethods> road_depot) {
    std::vector<bool> accepts = road_depot -> get_accepts_vector();
    for (int type = 0; type < accepts.size(); type++) {
        if (accepts[type]) {
            add_accept(type);
        }
    }
}

void RoadDepotWOMethods::add_accepts_from_depot(const RoadDepotWOMethods* road_depot) {
    std::vector<bool> accepts = road_depot -> get_accepts_vector();
    for (int type = 0; type < accepts.size(); type++) {
        if (accepts[type]) {
            add_accept(type);
        }
    }
}

void RoadDepotWOMethods::refresh_accepts() {
    StationWOMethods::refresh_accepts(); //Adds accepts from towns/factories/ect
    for (const auto &tile: other_road_depots) {
        Ref<RoadDepotWOMethods> road_depot =  TerminalMap::get_instance() -> get_terminal_as<RoadDepotWOMethods>(tile);
        if (road_depot.is_null()) continue;
        add_accepts_from_depot(road_depot);
    }
}

float RoadDepotWOMethods::get_local_price(int type) const {
    std::scoped_lock lock(m);
    return local_pricer -> get_local_price(type);
}

//Only called by brokers selling, Depot is buying
bool RoadDepotWOMethods::is_price_acceptable(int type, float pricePer) const {
    return is_price_acceptable_to_buy(type, pricePer);
}

//Depot is buying
bool RoadDepotWOMethods::is_price_acceptable_to_buy(int type, float pricePer) const {
    return get_local_price(type) >= pricePer * (1 + SERVICE_FEE);
}

//Depot is selling
bool RoadDepotWOMethods::is_price_acceptable_to_sell(int type, float pricePer) const {
    return get_local_price(type) <= pricePer / (1 + SERVICE_FEE);
}

void RoadDepotWOMethods::search_for_and_add_road_depots() { //Constructor runs this before being added to TerminalMap
    Ref<TerminalMap> terminal_map = TerminalMap::get_instance();
    std::priority_queue<
    godot_helpers::weighted_value<Vector2i>,
    std::vector<godot_helpers::weighted_value<Vector2i>>, /*vector on backend*/
    std::greater<godot_helpers::weighted_value<Vector2i>> /*Smallest in front*/
    > pq;
    std::unordered_set<Vector2i, godot_helpers::Vector2iHasher> s;
    s.insert(get_location());
    RoadMap* road_map = RoadMap::get_instance();
    auto push = [&pq](Vector2i tile, int weight) -> void {pq.push(godot_helpers::weighted_value<Vector2i>(tile, weight));};

    push(get_location(), 0);
    godot_helpers::weighted_value<Vector2i> curr;
    while (pq.size() > 0) {
        curr = pq.top();
        pq.pop();
        Ref<RoadDepotWOMethods> road_depot = terminal_map -> get_terminal_as<RoadDepotWOMethods>(curr.val);
        if (road_depot.is_valid() && road_depot -> get_location() != get_location()) {
            add_connected_road_depot(road_depot.ptr());
            road_depot -> add_connected_road_depot(this);
        }
        Array tiles = road_map -> get_surrounding_cells(curr.val);
        for (int i = 0; i < tiles.size(); i++) {
            if (tiles[i].get_type() != Variant::VECTOR2I) continue;
            Vector2i tile = tiles[i];
            if (!s.count(tile)) {
                s.insert(tile);
                int road_val = road_map -> get_road_value(tile);
                float weight = (road_val == 0 ? 2: road_val) + curr.weight;
                if (weight <= float(MAX_SUPPLY_DISTANCE)) {
                    push(tile, weight);
                }
            }
        }
    }
}

int RoadDepotWOMethods::get_desired_cargo(int type, float pricePer) const {
    if (does_accept(type)) {
        m.lock();
        int demand = local_pricer -> get_last_month_demand(type);
        m.unlock();
        return std::min(get_max_storage() - get_cargo_amount(type), std::min(get_amount_can_buy(pricePer), demand));
    }
    return 0;
}

float RoadDepotWOMethods::get_cash() const {
    if (get_player_owner() == 0) {
        return cash;
    } else {
        return Firm::get_cash();
    }
}

//For Testing

void RoadDepotWOMethods::add_cash(float amount) {
    if (get_player_owner() == 0) {
        m.lock();
        cash += amount;
        m.unlock();
    } else {
        Firm::add_cash(amount);
    }
}

void RoadDepotWOMethods::remove_cash(float amount) {
    if (get_player_owner() == 0) {
        m.lock();
        cash -= amount;
        m.unlock();
    } else {
        Firm::remove_cash(amount);
    }
}

bool RoadDepotWOMethods::is_connected_to_other_depot() const {
    return other_road_depots.size() != 0;
}

void RoadDepotWOMethods::day_tick() {
    distribute_cargo();
}

void RoadDepotWOMethods::month_tick() {
    refresh_accepts(); //Needs to happen monthly, since towns change orders
    search_for_and_add_road_depots();//Can be optimized later by running only when roads are placed within x tiles
    std::scoped_lock lock(m);
    local_pricer -> adjust_prices();
    
}