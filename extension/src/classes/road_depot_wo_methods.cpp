#include "road_depot_wo_methods.hpp"

void RoadDepotWOMethods::_bind_methods() {
    ClassDB::bind_method(D_METHOD("initialize", "new_location", "player_owner"), &RoadDepotWOMethods::initialize);

    // Expose methods to GDScript
    ClassDB::bind_method(D_METHOD("add_connected_road_depot", "road_depot"), &RoadDepotWOMethods::add_connected_road_depot);
    ClassDB::bind_method(D_METHOD("remove_connected_road_depot", "road_depot"), &RoadDepotWOMethods::remove_connected_road_depot);

    GDVIRTUAL_BIND(supply_armies);
}

RoadDepotWOMethods::RoadDepotWOMethods(): StationWOMethods() {}
RoadDepotWOMethods::RoadDepotWOMethods(Vector2i new_location, int player_owner): StationWOMethods(new_location, player_owner) {}

void RoadDepotWOMethods::initialize(Vector2i new_location, int player_owner) {
    StationWOMethods::initialize(new_location, player_owner);
}

void RoadDepotWOMethods::distribute_cargo() {
    for (const auto &[type, __]: storage) {
        distribute_type(type);
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
    int amount = std::min(amount_desired, get_cargo_amount(type) - cargo_sent);
    cargo_sent += amount;
    road_depot -> add_cargo(type, transfer_cargo(type, amount));
    
}

void RoadDepotWOMethods::add_connected_road_depot(RoadDepotWOMethods* new_road_depot) {
    ERR_FAIL_COND_MSG(other_road_depots.count(new_road_depot -> get_location()) == 0, "Already has a road depot there");
    other_road_depots[new_road_depot -> get_location()] = new_road_depot;
}

void RoadDepotWOMethods::remove_connected_road_depot(const RoadDepotWOMethods* new_road_depot) {
    ERR_FAIL_COND_MSG(other_road_depots.count(new_road_depot -> get_location()) != 0, "Never had a road depot there");
    other_road_depots.erase(new_road_depot -> get_location());
}

void RoadDepotWOMethods::day_tick() {
    distribute_cargo();
    StationWOMethods::day_tick();
}