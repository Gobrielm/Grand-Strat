#include "road_depot.hpp"
#include "broker.hpp"
#include "town.hpp"
#include "factory_template.hpp"
#include "../singletons/road_map.hpp"
#include "../singletons/cargo_info.hpp"
#include "../singletons/terminal_map.hpp"

#include <godot_cpp/core/class_db.hpp>

void RoadDepot::_bind_methods() {
    ClassDB::bind_static_method(get_class_static(), D_METHOD("create", "new_location", "player_owner"), &RoadDepot::create);

    ClassDB::bind_method(D_METHOD("month_tick"), &RoadDepot::month_tick);
}

Ref<RoadDepot> RoadDepot::create(const Vector2i new_location, const int player_owner) {
    return memnew(RoadDepot(new_location, player_owner));
}

RoadDepot::RoadDepot(): Firm() {}

RoadDepot::RoadDepot(Vector2i new_location, int player_owner): Firm(new_location, player_owner) {}

RoadDepot::~RoadDepot() {
    
}

void RoadDepot::add_connected_broker(Ref<Broker> broker) {
    std::scoped_lock lock(m);
    connected_brokers.insert(broker->get_location());
}

void RoadDepot::remove_connected_broker(const Ref<Broker> broker) {
    std::scoped_lock lock(m);
    connected_brokers.erase(broker->get_location());
}

void RoadDepot::add_connected_road_depot(const Vector2i road_depot_tile) {
    // ERR_FAIL_COND_MSG(other_road_depots.count(new_road_depot -> get_location()) != 0, "Already has a road depot there");
    if (other_road_depots.count(road_depot_tile) == 1) return; 
    m.lock();
    other_road_depots.insert(road_depot_tile);
    m.unlock();
}

void RoadDepot::remove_connected_road_depot(const Vector2i road_depot_tile) {
    // ERR_FAIL_COND_MSG(other_road_depots.count(new_road_depot -> get_location()) == 0, "Dones't has a road depot there");
    if (other_road_depots.count(road_depot_tile) == 0) return; 
    m.lock();
    other_road_depots.erase(road_depot_tile);
    m.unlock();
}


std::vector<Ref<Broker>> RoadDepot::get_available_brokers(int type) {
    Ref<TerminalMap> terminal_map = TerminalMap::get_instance();
    std::vector<Ref<Broker>> toReturn = get_available_local_brokers(type);
    for (const auto &tile: other_road_depots) {
        Ref<RoadDepot> road_depot = terminal_map->get_terminal_as<RoadDepot>(tile);
        if (road_depot.is_valid()) {
            for (Ref<Broker> broker: road_depot->get_available_local_brokers(type)) {
                toReturn.push_back(broker);
            }
        }
    }

    return toReturn;
}

std::vector<Ref<Broker>> RoadDepot::get_available_local_brokers(int type) {
    Ref<TerminalMap> terminal_map = TerminalMap::get_instance();
    std::vector<Ref<Broker>> toReturn;
    for (const auto &tile: connected_brokers) {
        Ref<Broker> broker = terminal_map->get_broker(tile);
        if (broker.is_valid() && broker->does_accept(type)) {
            toReturn.push_back(broker);
        }
    }

    return toReturn;
}

void RoadDepot::refresh_other_road_depots() {
    std::unordered_set<Vector2i, godot_helpers::Vector2iHasher> new_depots = get_reachable_road_depots();
    for (const Vector2i &tile: other_road_depots) {
        if (!new_depots.count(tile)) {
            remove_connected_road_depot(tile);
        }
    }

    for (auto it = new_depots.begin(); it != new_depots.end(); it++) {
        add_connected_road_depot(*it);
    }
}

std::unordered_set<Vector2i, godot_helpers::Vector2iHasher> RoadDepot::get_reachable_road_depots() {
    std::unordered_set<Vector2i, godot_helpers::Vector2iHasher> toReturn;
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
        Ref<RoadDepot> road_depot = terminal_map -> get_terminal_as<RoadDepot>(curr.val);
        if (road_depot.is_valid() && curr.val != get_location() && road_depot->get_player_owner() == get_player_owner()) { //Only add for same player/ai
            if (is_road_depot_valid(road_depot)) {
                toReturn.insert(curr.val);
            }
            // continue; //Cannot go beyond road depots
        }
        Array tiles = road_map -> get_surrounding_cells(curr.val);
        for (int i = 0; i < tiles.size(); i++) {
            if (tiles[i].get_type() != Variant::VECTOR2I) continue;
            Vector2i tile = tiles[i];
            if (!s.count(tile)) {
                s.insert(tile);
                int road_val = road_map -> get_road_value(tile);
                int weight = (road_val == 0 ? 11: 1) + curr.weight; //Use only roads
                if (weight <= (MAX_SUPPLY_DISTANCE)) {
                    push(tile, weight);
                }
            }
        }
    }
    return toReturn;
}

bool RoadDepot::is_road_depot_valid(Ref<RoadDepot> road_depot) const {
    std::unordered_set<int> supplies_needed; // Supplies this depot needs
    std::unordered_set<int> supplies_provided; // Supplies this depot needs
    
    Ref<TerminalMap> terminal_map = TerminalMap::get_instance();
    for (Vector2i tile: connected_brokers) {
        Ref<Town> town = terminal_map->get_terminal_as<Town>(tile);
        if (town.is_valid()) return true; 

        Ref<Broker> broker = terminal_map -> get_terminal_as<Broker>(tile);
        if (broker.is_null()) continue;
        int type = 0;
        for (bool valid: broker->get_accepts_vector()) {
            if (valid) supplies_needed.insert(type);
            type++;
        }

        Ref<FactoryTemplate> fact = terminal_map->get_terminal_as<FactoryTemplate>(tile);

        for (const auto& [type, __]: fact->outputs) {
            supplies_provided.insert(type);
        }
    }

    for (Vector2i tile: road_depot -> connected_brokers) {
        if (connected_brokers.count(tile)) continue; // If it has the same broker then ignore it
        Ref<Town> town = terminal_map->get_terminal_as<Town>(tile);
        if (town.is_valid()) return true; 

        Ref<Broker> broker = terminal_map -> get_terminal_as<Broker>(tile);
        if (broker.is_null()) continue;
        int type = 0;
        for (bool valid: broker->get_accepts_vector()) {
            if (valid && supplies_provided.count(type)) return true; // If other depot can accepts what this provides
            type++;
        }

        Ref<FactoryTemplate> fact = terminal_map->get_terminal_as<FactoryTemplate>(tile);

        for (const auto& [type, __]: fact->outputs) {
            if (supplies_needed.count(type)) return true;  // If other depot makes what this depot needs
        }
    }
    return false;
}


float RoadDepot::get_fee() {
    return FEE;
}

bool RoadDepot::is_connected_to_other_depot() const {
    std::scoped_lock lock(m);
    return connected_brokers.size() != 0;
}

void RoadDepot::month_tick() {}