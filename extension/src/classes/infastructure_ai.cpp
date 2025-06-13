#include "infastructure_ai.hpp"
#include "../singletons/province_manager.hpp"
#include "../singletons/terminal_map.hpp"
#include <godot_cpp/classes/script.hpp>

void InfastructureAi::_bind_methods() {
    ClassDB::bind_static_method(get_class_static(), D_METHOD("create", "p_country_id", "p_owner_id"), &InfastructureAi::create);

    ClassDB::bind_method(D_METHOD("month_tick"), &InfastructureAi::month_tick);
    ClassDB::bind_method(D_METHOD("run_until_nothing"), &InfastructureAi::run_until_nothing);
}

InfastructureAi* InfastructureAi::create(int p_country_id, int p_owner_id) {
    return memnew(InfastructureAi(p_country_id, p_owner_id));
}

InfastructureAi::InfastructureAi(): AiBase() {}

InfastructureAi::InfastructureAi(int p_country_id, int p_owner_id): AiBase(p_country_id, p_owner_id) {}

void InfastructureAi::build_roads() {
    std::vector<Vector2i> v = bfs_to_closest(get_stored_tile(), [](Vector2i tile) {
        return RoadMap::get_instance() -> get_road_value(tile) != 0 || TerminalMap::get_instance() -> is_town(tile);
    });
    
    auto start = v.begin();
    RoadMap* road_map = RoadMap::get_instance();
    if (TerminalMap::get_instance() -> is_town(v.front())) {
        road_map->place_road_depot(*start); //Place Depot
        start++;
    }

    for (auto it = start; it != v.end(); it++) {
        road_map->place_road(*it);
    }
}


void InfastructureAi::build_rails() {
    return;
    // std::vector<Vector2i> v = bfs_to_closest(get_stored_tile(), [](Vector2i tile) {
    //     return RoadMap::get_instance() -> get_road_value(tile) != 0 || TerminalMap::get_instance() -> is_town(tile); //TODO: No rails available to c++
    // });
    
    // auto start = v.begin();
    // RoadMap* road_map = RoadMap::get_instance();
    // if (TerminalMap::get_instance() -> is_town(v.front())) {
    //     road_map->place_road_depot(*start); //Place Depot
    //     start++;
    // }

    // for (auto it = start; it != v.end(); it++) {
    //     road_map->place_road(*it);
    // }
}

void InfastructureAi::build_road_depot() {
    TileMapLayer* main_map = TerminalMap::get_instance() -> get_main_map();
    Array tiles = main_map->get_surrounding_cells(get_stored_tile());
    tiles.shuffle();
    for (int i = 0; i < tiles.size(); i++) {
        if (tiles[i].get_type() != Variant::VECTOR2I) continue;
        Vector2i tile = tiles[i];
        if (TerminalMap::get_instance() -> is_tile_available(tile)) {
            TileMapLayer* cargo_map = TerminalMap::get_instance() -> get_cargo_map();
            cargo_map->call("place_road_depot", tile, get_owner_id());
        }
    }
}

void InfastructureAi::build_train_station() {
    
}

template<typename Predicate>
std::vector<Vector2i> InfastructureAi::bfs_to_closest(Vector2i start, Predicate closest) {
    Ref<TerminalMap> terminal_map = TerminalMap::get_instance();
    TileMapLayer* main_map = terminal_map -> get_main_map();
    RoadMap* road_map = RoadMap::get_instance();
    std::queue<Vector2i> q;
    std::unordered_set<Vector2i, godot_helpers::Vector2iHasher> s;
    std::unordered_map<Vector2i, Vector2i, godot_helpers::Vector2iHasher> tile_to_prev;
    q.push(start);
    s.insert(start);
    Vector2i curr;
    bool found = false;
    while (q.size() != 0 && !found) {
        if (s.size() > 1000) return std::vector<Vector2i>();
        curr = q.front();
        q.pop();
        
        Array tiles = main_map->get_surrounding_cells(curr);
        for (int i = 0; i < tiles.size(); i++) {
            if (tiles[i].get_type() != Variant::VECTOR2I) continue;
            Vector2i tile = tiles[i];
            if (!TerminalMap::get_instance() -> is_tile_traversable(tile, false) || !is_tile_owned(tile)) continue;
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
    if (found) {
        toReturn.push_back(curr);
        //Potentially build road depot if connecting to town
        while (tile_to_prev.count(curr)) {
            curr = tile_to_prev[curr];
            toReturn.push_back(curr);
        }
    }
    return toReturn;
}
bool InfastructureAi::is_tile_owned(Vector2i tile) {
    Ref<ProvinceManager> province_manager = ProvinceManager::get_instance();
    return province_manager -> get_province(province_manager -> get_province_id(tile)) -> get_country_id() == get_country_id();
}   

AiActions InfastructureAi::decide_action() {
    AiActions first_action = check_for_unconnected_stations();
    if (first_action != AiActions::nothing) return first_action; 

    int max_weight = check_for_unconnected_buildings();
    if (max_weight > ROAD_THRESHOLD) {
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
            if (terminal_map -> is_road_depot(tile) && !(terminal_map -> get_terminal_as<RoadDepotWOMethods>(tile)->is_connected_to_other_depot())) {
                set_stored_tile(tile);
                return AiActions::build_roads;
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
            if (has_connected_station(tile)) continue; 
            int weight = get_trade_weight(tile);
            if (weight > highest_weight) {
                highest_weight = weight;
                set_stored_tile(tile);
            }
        }
    }
    return highest_weight;
}

bool InfastructureAi::has_connected_station(Vector2i tile) const {
    return TerminalMap::get_instance()->get_station(tile)->get_connected_broker_locations().size() != 0;
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
    } else if (action == AiActions::build_road_depot) {
        build_road_depot();
    } else if (action == AiActions::build_roads) {
        build_roads();
    } else if (action == AiActions::build_train_station) {
        build_train_station();
    } else if (action == AiActions::build_rails) {
        build_rails();
    }
}

void InfastructureAi::run_until_nothing() {
    int num = 0;
    while (num < 20000) {
        num++;
        AiActions action = decide_action();
        if (action == AiActions::nothing) {
            return;
        } else if (action == AiActions::build_road_depot) {
            build_road_depot();
        } else if (action == AiActions::build_roads) {
            build_roads();
        }
    }
}