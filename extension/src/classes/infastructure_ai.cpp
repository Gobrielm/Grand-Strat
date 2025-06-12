#include "infastructure_ai.hpp"
#include "../singletons/province_manager.hpp"
#include "../singletons/terminal_map.hpp"

void InfastructureAi::_bind_methods() {

}

InfastructureAi::InfastructureAi(): AiBase() {}

InfastructureAi::InfastructureAi(int p_country_id, int p_owner_id): AiBase(p_country_id, p_owner_id) {}

void InfastructureAi::build_roads() {

}


void InfastructureAi::build_rails() {

}

void InfastructureAi::build_road_depot() {
    std::vector<Vector2i> v = bfs_to_closest(get_stored_tile(), [](Vector2i tile) {
        return RoadMap::get_instance() -> get_road_value(tile) != 0 || TerminalMap::get_instance() -> is_town(tile);
    });
    
    auto start = v.begin();
    Ref<RoadMap> road_map = RoadMap::get_instance();
    if (TerminalMap::get_instance() -> is_town(v.front())) {
        road_map->place_road_depot(*start); //Place Depot
        start++;
    }

    for (auto it = start; it != v.end(); it++) {
        road_map->place_road(*it);
    }
}

void InfastructureAi::build_train_station() {

}

template<typename Predicate>
std::vector<Vector2i> InfastructureAi::bfs_to_closest(Vector2i start, Predicate closest) {
    Ref<TerminalMap> terminal_map = TerminalMap::get_instance();
    TileMapLayer* main_map = terminal_map -> get_main_map();
    Ref<RoadMap> road_map = RoadMap::get_instance();
    std::queue<Vector2i> q;
    std::unordered_set<Vector2i, godot_helpers::Vector2iHasher> s;
    std::unordered_map<Vector2i, Vector2i, godot_helpers::Vector2iHasher> tile_to_prev;
    q.push(start);
    s.insert(start);
    Vector2i curr;
    bool found = false;
    while (q.size() != 0 && !found) {
        if (s.size() > 1000) return;
        curr = q.front();
        q.pop();
        
        Array tiles = main_map->get_surrounding_cells(curr);
        for (int i = 0; i < tiles.size(); i++) {
            if (tiles[i].get_type() != Variant::VECTOR2I) continue;
            Vector2i tile = tiles[i];
            if (!TerminalMap::get_instance() -> is_tile_traversable(tile, false)) continue;
            if (s.count(tile)) continue;


            q.push(tile);
            s.insert(tile);
            tile_to_prev[tile] = curr;
            if (closest(tile)) {
                found = true;
                curr = tile;
                break;
            }
        }
    }
    std::vector<Vector2i> toReturn;
    if found {
        toReturn.push_back(curr);
        //Potentially build road depot if connecting to town
        while (tile_to_prev.count(curr)) {
            curr = tile_to_prev[curr];
            toReturn.push_back(curr);
        }
    }
    return toReturn;
}

AiActions InfastructureAi::decide_action() {
    AiActions first_action = check_for_unconnected_stations();
    if (first_action != AiActions::nothing) return first_action; 

    int max_weight = check_for_unconnected_buildings();
    if (max_weight > RAIL_THRESHOLD) {
        return AiActions::build_train_station;
    } else if (max_weight > ROAD_THRESHOLD) {
        return AiActions::build_road_depot;
    }
    return AiActions::nothing;
}

AiActions InfastructureAi::check_for_unconnected_stations() {
    Ref<TerminalMap> terminal_map = TerminalMap::get_instance();
    Ref<ProvinceManager> province_manager = ProvinceManager::get_instance();
    std::unordered_set<int> prov_ids = province_manager->get_country_provinces(get_country_id());
    for (int prov_id: prov_ids) {
        Province* province = province_manager->get_province(prov_id);
        for (const Vector2i& tile: province->get_terminal_tiles_set()) {
            if (terminal_map -> is_road_depot(tile)) {
                set_stored_tile(tile);
                return AiActions::build_roads;
            } else if (terminal_map -> is_station(tile)) {
                set_stored_tile(tile);
                return AiActions::build_rails;
            }
        }
    }
    return AiActions::nothing;
}

int InfastructureAi::check_for_unconnected_buildings() {
    int highest_weight = 0;
    Ref<ProvinceManager> province_manager = ProvinceManager::get_instance();
    std::unordered_set<int> prov_ids = province_manager->get_country_provinces(get_country_id());
    for (int prov_id: prov_ids) {
        Province* province = province_manager->get_province(prov_id);
        for (const Vector2i& tile: province->get_terminal_tiles_set()) {
            int weight = get_trade_weight(tile);
            if (weight > highest_weight) {
                highest_weight = weight;
                set_stored_tile(tile);
            }
        }
    }
    return highest_weight;
}

int InfastructureAi::get_trade_weight(Vector2i tile) {
    Ref<TerminalMap> terminal_map = TerminalMap::get_instance();
    Ref<Factory> fact = terminal_map -> get_terminal_as<Factory>(tile);
    if (fact.is_valid()) {
        return fact->get_level();
    }
    Ref<Town> town = terminal_map -> get_terminal_as<Town>(tile);
    if (town.is_valid()) {
        return round(town -> get_total_pops() / 3);
    }
    return 0;
}

void InfastructureAi::month_tick() {
    AiActions action = decide_action();
    if (action == AiActions::nothing) {
        return;
    }
}
