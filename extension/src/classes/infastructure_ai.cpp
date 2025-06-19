#include <queue>
#include "infastructure_ai.hpp"
#include "factory.hpp"
#include "town.hpp"
#include "../singletons/province_manager.hpp"
#include "../singletons/terminal_map.hpp"
#include "../singletons/road_map.hpp"

void InfastructureAi::_bind_methods() {
    ClassDB::bind_static_method(get_class_static(), D_METHOD("create", "p_country_id", "p_owner_id"), &InfastructureAi::create);

    ClassDB::bind_method(D_METHOD("connect_towns"), &InfastructureAi::connect_towns);
}

InfastructureAi* InfastructureAi::create(int p_country_id, int p_owner_id) {
    return memnew(InfastructureAi(p_country_id, p_owner_id));
}

InfastructureAi::InfastructureAi(): AiBase() {}

InfastructureAi::InfastructureAi(int p_country_id, int p_owner_id): AiBase(p_country_id, p_owner_id) {
    road_map = RoadMap::get_instance();
    cargo_map = TerminalMap::get_instance()->get_cargo_map();
}

void InfastructureAi::build_road_depot() {
    Ref<TerminalMap> terminal_map = TerminalMap::get_instance();
    TileMapLayer* main_map = terminal_map -> get_main_map();
    Array tiles = main_map->get_surrounding_cells(get_stored_tile());
    tiles.shuffle();
    for (int i = 0; i < tiles.size(); i++) {
        if (tiles[i].get_type() != Variant::VECTOR2I) continue;
        Vector2i tile = tiles[i];
        if (terminal_map -> is_tile_available(tile)) {
            TileMapLayer* cargo_map = terminal_map -> get_cargo_map();
            cargo_map->call("place_road_depot", tile, get_owner_id());
            print_line("place road depot");
            break;
        }
    }
}

void InfastructureAi::build_roads() {
    Ref<TerminalMap> terminal_map = TerminalMap::get_instance();
    std::vector<Vector2i> v = bfs_to_closest(get_stored_tile(), [](Vector2i tile) {
        return RoadMap::get_instance() -> get_road_value(tile) != 0 || TerminalMap::get_instance() -> is_town(tile);
    });
    if (v.size() == 0) return;
    auto start = v.begin();
    RoadMap* road_map = RoadMap::get_instance();
    
    std::advance(start, 1);

    for (auto it = start; it != v.end(); it++) {
        road_map->place_road(*it);
    }

    if (terminal_map -> is_town(v.front())) {
        start = v.begin();
        std::advance(start, 1);
        TileMapLayer* cargo_map = terminal_map->get_cargo_map();
        cargo_map->call("place_road_depot", *start, get_owner_id());
        road_map->place_road_depot(*start); //Place Depot
    }
}

std::vector<Vector2i> InfastructureAi::bfs_to_closest(Vector2i start, bool(*f)(Vector2i)) {
    Ref<TerminalMap> terminal_map = TerminalMap::get_instance();
    std::queue<Vector2i> q;
    std::unordered_set<Vector2i, godot_helpers::Vector2iHasher> s;
    std::unordered_map<Vector2i, Vector2i, godot_helpers::Vector2iHasher> tile_to_prev;
    q.push(start);
    s.insert(start);

    TileMapLayer* main_map = terminal_map->get_main_map();
    Array near_tiles = main_map->get_surrounding_cells(start);
    for (int i = 0; i < near_tiles.size(); i++) {
        if (near_tiles[i].get_type() != Variant::VECTOR2I) continue;
        Vector2i tile = near_tiles[i];
        if (f(tile)) s.insert(tile);
    }
    
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
            if (!terminal_map -> is_tile_traversable(tile, true) || !is_tile_owned(tile)) continue;
            if (s.count(tile)) continue;


            q.push(tile);
            s.insert(tile);
            tile_to_prev[tile] = curr;
            if (f(tile)) {
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
            if (tile_to_prev.count(curr) && tile_to_prev[curr] == start) {
                break;
            }
        }
    }
    return toReturn;
}

void InfastructureAi::check_for_unconnected_stations() {
    Ref<ProvinceManager> province_manager = ProvinceManager::get_instance();
    Ref<TerminalMap> terminal_map = TerminalMap::get_instance();
    std::unordered_set<int> prov_ids = province_manager->get_country_provinces(get_country_id());
    for (int prov_id: prov_ids) {
        Province* province = province_manager->get_province(prov_id);
        for (const Vector2i& tile: province->get_terminal_tiles_set()) {
            if (terminal_map -> is_road_depot(tile) && !(terminal_map -> get_terminal_as<RoadDepot>(tile)->is_connected_to_other_depot())) {
                set_stored_tile(tile);
            }
        }
    }
}

int InfastructureAi::check_for_unconnected_buildings() {
    Ref<ProvinceManager> province_manager = ProvinceManager::get_instance();
    Ref<TerminalMap> terminal_map = TerminalMap::get_instance();
    int highest_weight = 0;
    std::unordered_set<int> prov_ids = province_manager->get_country_provinces(get_country_id());
    for (int prov_id: prov_ids) {
        Province* province = province_manager->get_province(prov_id);
        for (const Vector2i& tile: province->get_terminal_tiles_set()) {
            if (terminal_map->is_station(tile) || has_connected_station(tile)) continue; 
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
    Ref<TerminalMap> terminal_map = TerminalMap::get_instance();
    Array tiles = terminal_map->get_main_map()->get_surrounding_cells(tile);
    for (int i = 0; i < tiles.size(); i++) {
        Vector2i cell = tiles[i];
        if (terminal_map->is_station(cell)) return true;
        
    }
    return false;
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


void InfastructureAi::connect_towns() {
    Ref<ProvinceManager> province_manager = ProvinceManager::get_instance();
    Ref<TerminalMap> terminal_map = TerminalMap::get_instance();
    std::unordered_set<Vector2i, godot_helpers::Vector2iHasher> towns_to_service;

    std::unordered_set<int> prov_ids = province_manager->get_country_provinces(get_country_id());
    for (int prov_id: prov_ids) {
        Province* province = province_manager->get_province(prov_id);
        for (const Vector2i& tile: province->get_terminal_tiles_set()) {
            if (terminal_map->is_town(tile)) {
                towns_to_service.insert(tile);
            }
        }
    }

    while (towns_to_service.size() != 0) {
        Ref<Town> town = terminal_map->get_town(*towns_to_service.begin());
        Vector2i tile = town->get_location();
        auto lbda = [](Vector2i tile) {
            return RoadMap::get_instance() -> get_road_value(tile) != 0 || TerminalMap::get_instance() -> is_town(tile);
        };

        std::vector<Vector2i> v = bfs_to_closest(town->get_location(), lbda);

        if (v.size() != 0 && v.size() <= (RoadDepot::MAX_SUPPLY_DISTANCE + 1)) { //Doesn't work with supply

            if (!terminal_map->is_terminal(v.back())) {
                cargo_map->call("place_road_depot", v.back(), get_owner_id());
            }

            auto start = v.begin();
            
            std::advance(start, 1);

            for (auto it = start; it != v.end(); it++) {
                road_map->place_road(*it);
            }

            if (terminal_map -> is_town(v.front())) {
                start = v.begin();
                if (towns_to_service.count(*start)) towns_to_service.erase(*start);
                
                std::advance(start, 1);
                if (!terminal_map->is_terminal(*start)) {
                    cargo_map->call("place_road_depot", *start, get_owner_id());
                }
            }
        }
        towns_to_service.erase(tile);
    }
}

void InfastructureAi::connect_factories() {
    Ref<ProvinceManager> province_manager = ProvinceManager::get_instance();
    Ref<TerminalMap> terminal_map = TerminalMap::get_instance();
    std::unordered_set<Vector2i, godot_helpers::Vector2iHasher> factories_to_check;

    std::unordered_set<int> prov_ids = province_manager->get_country_provinces(get_country_id());
    for (int prov_id: prov_ids) {
        Province* province = province_manager->get_province(prov_id);
        for (const Vector2i& tile: province->get_terminal_tiles_set()) {
            if (terminal_map->is_factory(tile)) {
                factories_to_check.insert(tile);
            }
        }
    }

    while (factories_to_check.size() != 0) {
        const Vector2i tile = *factories_to_check.begin();
        Ref<Factory> factory = terminal_map->get_terminal_as<Factory>(tile);
        connect_factory(factory);
        factories_to_check.erase(tile);
    }
}

void InfastructureAi::connect_factory(Ref<Factory> factory) {
    Ref<TerminalMap> terminal_map = TerminalMap::get_instance();
    Array tiles = terminal_map->get_main_map()->get_surrounding_cells(factory->get_location());
    for (int i = 0; i < tiles.size(); i++) {
        Vector2i cell = tiles[i];
        if (terminal_map -> is_road_depot(cell)) return;
    }


    Vector2i tile = factory->get_location();
    
    auto lbda = [](Vector2i tile) {
        return RoadMap::get_instance() -> get_road_value(tile) != 0;
    };

    std::vector<Vector2i> v = bfs_to_closest(tile, lbda);

    if (v.size() != 0 && v.size() <= (RoadDepot::MAX_SUPPLY_DISTANCE + 1)) { //Doesn't work with supply

        if (!terminal_map->is_terminal(v.back())) {
            cargo_map->call("place_road_depot", v.back(), get_owner_id());
        }

        auto start = v.begin();
        
        std::advance(start, 1);

        for (auto it = start; it != v.end(); it++) {
            road_map->place_road(*it);
        }

        if (terminal_map -> is_town(v.front())) {
            start = v.begin();
            
            std::advance(start, 1);
            if (!terminal_map->is_terminal(*start)) {
                cargo_map->call("place_road_depot", *start, get_owner_id());
            }
        }
    }
    
}