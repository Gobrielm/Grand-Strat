#include "road_depot_wo_methods.hpp"

//TODO: Problems with road depots selling in between each other, and depots selling to towns and towns selling right back
//Build a custom inherited local pricer that uses supply / demand to have its own local price, then just sell profitably, and will go in the right direction
//Then 'island' depots can still trade and have local market, good for supply army method.

void RoadDepotWOMethods::_bind_methods() {
    ClassDB::bind_method(D_METHOD("initialize", "new_location", "player_owner"), &RoadDepotWOMethods::initialize);

    // Expose methods to GDScript
    ClassDB::bind_method(D_METHOD("add_connected_road_depot", "road_depot"), &RoadDepotWOMethods::add_connected_road_depot);
    ClassDB::bind_method(D_METHOD("remove_connected_road_depot", "road_depot"), &RoadDepotWOMethods::remove_connected_road_depot);

    ClassDB::bind_method(D_METHOD("month_tick"), &RoadDepotWOMethods::month_tick);

    GDVIRTUAL_BIND(supply_armies);
    GDVIRTUAL_BIND(get_road_depot, "tile");
}

RoadDepotWOMethods::RoadDepotWOMethods(): StationWOMethods() {}
RoadDepotWOMethods::RoadDepotWOMethods(Vector2i new_location, int player_owner): StationWOMethods(new_location, player_owner) {search_for_and_add_road_depots();}

RoadDepotWOMethods::~RoadDepotWOMethods() {
    for (const auto &[__, road_depot]: other_road_depots) {
        road_depot -> remove_connected_road_depot(this);
    }
}

//Called by godot
void RoadDepotWOMethods::initialize(Vector2i new_location, int player_owner) {
    StationWOMethods::initialize(new_location, player_owner);
    depot_accepts.resize(CargoInfo::get_instance() -> get_base_prices().size());
    search_for_and_add_road_depots();
}

void RoadDepotWOMethods::refresh_depot_accepts() {
    for (int type = 0; type < depot_accepts.size(); type++) {
        if (does_accept(type)) { // If depot accepts then it is sink depot
            depot_accepts[type] = 0;
        } else {
            for (const auto &[__, depot]: other_road_depots) {
                int val = depot -> get_depot_accept(type) + 1;
                if (val == 0) continue; // Has no connection to it
                if (depot_accepts[type] > val || depot_accepts[type] == -1) { // Found a shorter path or created a new one
                    depot_accepts[type] = val;
                }
            }
        }
    }
}

int RoadDepotWOMethods::get_depot_accept(int type) const {
    return depot_accepts[type];
}


void RoadDepotWOMethods::distribute_cargo() {
    cargo_sent = 0;
    for (const auto &[type, amount]: storage) {
        if (amount == 0) continue;
        distribute_type(type);
    }	
}

void RoadDepotWOMethods::distribute_type(int type) {
    //Prioritize local before sending onward
    for (const auto &[__, broker]: connected_brokers) {
        if (broker -> does_accept(type) && broker -> get_player_owner() == get_player_owner()) {
			distribute_type_to_broker(type, broker);
        }
    }

    for (const auto &[__, road_depot]: other_road_depots) {
        if (cargo_sent == MAX_THROUGHPUT) break;
        //Must be owned by same person
		if (road_depot -> does_accept(type) && road_depot -> get_player_owner() == get_player_owner()) {
            distribute_type_to_road_depot(type, road_depot);
        }
    }
}

void RoadDepotWOMethods::distribute_type_to_road_depot(int type, RoadDepotWOMethods* road_depot) {
    int amount_desired = road_depot -> get_desired_cargo_from_train(type);
    int amount = std::min(amount_desired, std::min(get_cargo_amount(type), MAX_THROUGHPUT - cargo_sent));
    cargo_sent += amount;
    road_depot -> add_cargo(type, transfer_cargo(type, amount));
}

void RoadDepotWOMethods::distribute_type_to_broker(int type, Broker* broker) {
    int amount_desired = broker -> get_desired_cargo_from_train(type);
    int amount = std::min(amount_desired, get_cargo_amount(type));
    broker -> add_cargo(type, transfer_cargo(type, amount));
}

void RoadDepotWOMethods::add_connected_broker(Broker* broker) {
    StationWOMethods::add_connected_broker(broker);
    for (const auto &[__, road_depot]: other_road_depots) {
        road_depot -> add_accepts_from_depot(this);
    }
}

void RoadDepotWOMethods::remove_connected_broker(const Broker* broker) {
    StationWOMethods::remove_connected_broker(broker);
    for (const auto &[__, road_depot]: other_road_depots) {
        road_depot -> refresh_accepts();
    }
}

void RoadDepotWOMethods::add_connected_road_depot(RoadDepotWOMethods* new_road_depot) {
    ERR_FAIL_COND_MSG(other_road_depots.count(new_road_depot -> get_location()) != 0, "Already has a road depot there");
    other_road_depots[new_road_depot -> get_location()] = new_road_depot;
    add_accepts_from_depot(new_road_depot);
}

void RoadDepotWOMethods::remove_connected_road_depot(const RoadDepotWOMethods* new_road_depot) {
    ERR_FAIL_COND_MSG(other_road_depots.count(new_road_depot -> get_location()) == 0, "Never had a road depot there");
    other_road_depots.erase(new_road_depot -> get_location());
    refresh_accepts();
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
    for (const auto &[__, road_depot]: other_road_depots) {
        add_accepts_from_depot(road_depot);
    }
}

//Depot is buying
bool RoadDepotWOMethods::is_price_acceptable(int type, float pricePer) const {
    return true;
    // get_local_price(type) >= pricePer;
}


void RoadDepotWOMethods::search_for_and_add_road_depots() {
    std::priority_queue<
    godot_helpers::weighted_value<Vector2i>,
    std::vector<godot_helpers::weighted_value<Vector2i>>, /*vector on backend*/
    std::greater<godot_helpers::weighted_value<Vector2i>> /*Smallest in front*/
    > pq;
    std::unordered_set<Vector2i, godot_helpers::Vector2iHasher> s;
    s.insert(get_location());
    RoadMap* road_map = RoadMap::get_instance();
    auto push = [&pq](Vector2i vector, int weight) -> void {pq.push(godot_helpers::weighted_value<Vector2i>(vector, weight));};

    push(get_location(), 0);
    godot_helpers::weighted_value<Vector2i> curr;
    while (pq.size() > 0) {
        curr = pq.top();
        pq.pop();
        RoadDepotWOMethods* road_depot = get_road_depot(curr.val);
        if (road_depot != nullptr && road_depot != this) {
            add_connected_road_depot(road_depot);
            road_depot -> add_connected_road_depot(this);
        }

        Array tiles = road_map -> get_surrounding_cells(curr.val);
        for (int i = 0; i < tiles.size(); i++) {
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
        return std::min(get_max_storage() - get_cargo_amount(type), get_amount_can_buy(pricePer));
    }
    return 0;
}

RoadDepotWOMethods* RoadDepotWOMethods::get_road_depot(Vector2i tile) const {
    RoadDepotWOMethods* result = nullptr;
    if (GDVIRTUAL_CALL(get_road_depot, tile, result)) {
        return result;
    }
    ERR_FAIL_V_MSG(nullptr, "Not implemented");
}

float RoadDepotWOMethods::get_cash() const {
    if (get_player_owner() == 0) {
        return 1000;
    } else {
        return Firm::get_cash();
    }
}

//For Testing

void RoadDepotWOMethods::add_cash(float amount) {}

void RoadDepotWOMethods::remove_cash(float amount) {}

void RoadDepotWOMethods::day_tick() {
    distribute_cargo();
}

void RoadDepotWOMethods::month_tick() {
    refresh_accepts(); //Needs to happen monthly, since towns change orders
}