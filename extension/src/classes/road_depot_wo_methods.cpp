#include "road_depot_wo_methods.hpp"

void RoadDepotWOMethods::_bind_methods() {
    ClassDB::bind_method(D_METHOD("initialize", "new_location", "player_owner"), &RoadDepotWOMethods::initialize);

    // Expose methods to GDScript
    ClassDB::bind_method(D_METHOD("add_connected_road_depot", "road_depot"), &RoadDepotWOMethods::add_connected_road_depot);
    ClassDB::bind_method(D_METHOD("remove_connected_road_depot", "road_depot"), &RoadDepotWOMethods::remove_connected_road_depot);

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
    search_for_and_add_road_depots();
}

void RoadDepotWOMethods::distribute_cargo() {
    for (const auto &[type, __]: storage) {
        distribute_type(type);
        if (cargo_sent == MAX_THROUGHPUT) {
            return;
        }
    }	
}

void RoadDepotWOMethods::distribute_type(int type) {
    for (const auto &[tile, road_depot]: other_road_depots) {
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

void RoadDepotWOMethods::add_connected_road_depot(RoadDepotWOMethods* new_road_depot) {
    ERR_FAIL_COND_MSG(other_road_depots.count(new_road_depot -> get_location()) != 0, "Already has a road depot there");
    other_road_depots[new_road_depot -> get_location()] = new_road_depot;
}

void RoadDepotWOMethods::remove_connected_road_depot(const RoadDepotWOMethods* new_road_depot) {
    ERR_FAIL_COND_MSG(other_road_depots.count(new_road_depot -> get_location()) == 0, "Never had a road depot there");
    other_road_depots.erase(new_road_depot -> get_location());
}


void RoadDepotWOMethods::search_for_and_add_road_depots() {
    std::priority_queue<godot_helpers::weighted_value<Vector2i>> pq;
    std::unordered_set<Vector2i, godot_helpers::Vector2iHasher> s;
    s.insert(get_location());
    RoadMap* road_map = RoadMap::get_instance();
    auto push = [&pq](Vector2i vector, int weight) -> void {pq.push(godot_helpers::weighted_value<Vector2i>(vector, weight));};

    push(get_location(), 0);
    godot_helpers::weighted_value<Vector2i> curr;
    while (pq.size() != 0) {
        curr = pq.top();
        pq.pop();
        RoadDepotWOMethods* road_depot = get_road_depot(curr.val);
        if (road_depot != nullptr) {
            add_connected_road_depot(road_depot);
            road_depot -> add_connected_road_depot(this);
        }

        Array tiles = road_map -> get_surrounding_cells(curr.val);
        for (int i = 0; i < 6; i++) {
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

RoadDepotWOMethods* RoadDepotWOMethods::get_road_depot(Vector2i tile) const {
    RoadDepotWOMethods* result = nullptr;
    if (GDVIRTUAL_CALL(get_road_depot, tile, result)) {
        return result;
    }
    ERR_FAIL_V_MSG(nullptr, "Not implemented");
}

void RoadDepotWOMethods::day_tick() {
    distribute_cargo();
    StationWOMethods::day_tick();
}