#include "initial_builder.hpp"
#include <algorithm>
#include <random>
#include "../singletons/province_manager.hpp"
#include "../singletons/terminal_map.hpp"

void InitialBuilder::_bind_methods() {
    ClassDB::bind_static_method(get_class_static(), D_METHOD("create", "p_country_id"), &InitialBuilder::create);

    ClassDB::bind_method(D_METHOD("build_initital_factories"), &InitialBuilder::build_initital_factories);
}

bool InitialBuilder::does_have_money_for_investment() {
    return true;
}

InitialBuilder* InitialBuilder::create(int p_country_id) {
    return memnew(InitialBuilder(p_country_id));
}

InitialBuilder::InitialBuilder(): CompanyAi() {}
InitialBuilder::InitialBuilder(int p_country_id): CompanyAi(p_country_id, 0) {}

void InitialBuilder::build_initital_factories() {
    Ref<ProvinceManager> province_manager = ProvinceManager::get_instance();
    Ref<TerminalMap> terminal_map = TerminalMap::get_instance();
    std::unordered_set<int> prov_ids = province_manager->get_country_provinces(get_country_id());
    
    for (const int &prov_id: prov_ids) {
        Province* province = province_manager->get_province(prov_id);
        build_factory_type(CargoInfo::get_instance()->get_cargo_type("grain"), province);
        build_factory_type(CargoInfo::get_instance()->get_cargo_type("wood"), province);
    }
    build_and_connect_depots();
}

void InitialBuilder::build_factory_type(int type, Province* province) {
    Ref<TerminalMap> terminal_map = TerminalMap::get_instance();
    for (const Vector2i &tile: province->get_tiles_vector()) {
        if (TerminalMap::get_instance()->is_tile_available(tile)) {
            int cargo_val = get_cargo_value_of_tile(tile, type);
            int chance = 10;

            if (cargo_val != 0 && rand() % chance == 0) {
                Array recipe;
                recipe.push_back(Dictionary());
                Dictionary d;
                d[type] = 1;

                recipe.push_back(d);
                int mult = std::min(rand() % 5, cargo_val);
                cargo_map->call("create_factory", get_owner_id(), tile, recipe, mult);
            }
        }
    }
}

void InitialBuilder::build_and_connect_depots() {
    Ref<ProvinceManager> province_manager = ProvinceManager::get_instance();
    Ref<TerminalMap> terminal_map = TerminalMap::get_instance();
    std::unordered_set<int> prov_ids = province_manager->get_country_provinces(get_country_id());
    
    for (const int &prov_id: prov_ids) {
        Province* province = province_manager->get_province(prov_id);
        for (const Vector2i &tile: province->get_terminal_tiles_set()) {
            if (is_tile_adjacent_to_depot(tile)) continue;
            Ref<FactoryTemplate> factory = terminal_map->get_terminal_as<FactoryTemplate>(tile);
            if (factory.is_null()) continue;
            
            place_most_connected_depot(tile);
        }
    }
}

void InitialBuilder::place_most_connected_depot(const Vector2i &center) {
    Vector2i best_cell;
    int highest_score = 0;

    Array cells = TerminalMap::get_instance()->get_main_map()->get_surrounding_cells(center);
    for (int i = 0; i < cells.size(); i++) {
        Vector2i cell = cells[i];
        if (!TerminalMap::get_instance()->is_tile_available(cell)) continue;
        int count = count_adjacent_factories(cell);
        if (count > highest_score) {
            highest_score = count;
            best_cell = cell;
        }
    }

    if (best_cell != Vector2i(0, 0)) {
        place_depot(best_cell);
        connect_road_depot(best_cell);
    }
}

int InitialBuilder::count_adjacent_factories(const Vector2i &center) const {
    int count = 0;
    Ref<TerminalMap> terminal_map = TerminalMap::get_instance();
    Array tiles = terminal_map->get_main_map()->get_surrounding_cells(center);
    for (int i = 0; i < tiles.size(); i++) {
        Vector2i tile = tiles[i];
        Ref<FactoryTemplate> factory = terminal_map->get_terminal_as<FactoryTemplate>(tile);
        if (factory.is_valid()) count++;
    }
    return count;
}

int InitialBuilder::place_group_of_factories(const Vector2i &center) {
    int built = 0;
    TileMapLayer* main_map = TerminalMap::get_instance()->get_main_map();
    Array tiles = main_map->get_surrounding_cells(center);
    for (int i = 0; i < tiles.size(); i++) {
        Vector2i tile = tiles[i];
        if (TerminalMap::get_instance()->is_tile_available(tile)) {
            int grain_val = get_cargo_value_of_tile(tile, "grain");
            int wood_val = get_cargo_value_of_tile(tile, "wood");

            if (rand() % 4 == 0) {
                Array recipe;
                recipe.push_back(Dictionary());
                Dictionary d;
                if (grain_val > wood_val) d[CargoInfo::get_instance()->get_cargo_type("grain")] = 1;
                else d[CargoInfo::get_instance()->get_cargo_type("wood")] = 1;

                recipe.push_back(d);
                int mult = std::min(5 + rand() % 10, std::max(grain_val, wood_val));
                cargo_map->call("create_factory", get_owner_id(), tile, recipe, mult);
                built += mult;
            }
        }
    }
    if (built != 0) {
        place_depot(center);
        connect_road_depot(center);
        //Find and connect to closest town
    }
    return built;
}

void InitialBuilder::connect_road_depot(const Vector2i &depot) {
    Ref<TerminalMap> terminal_map = TerminalMap::get_instance();
    std::vector<Vector2i> v = bfs_to_closest(depot, [](Vector2i tile) {
        return RoadMap::get_instance() -> get_road_value(tile) != 0 || TerminalMap::get_instance() -> is_town(tile);
    });
    if (v.size() == 0) return;
    auto start = v.begin();
    RoadMap* road_map = RoadMap::get_instance();
    
    std::advance(start, 1);

    for (auto it = start; it != v.end(); it++) {
        road_map->place_road(*it);
    }

    //Places depot at town
    if (terminal_map -> is_town(v.front())) {
        start = v.begin();
        std::advance(start, 1);
        if (terminal_map->is_tile_available(*start)) place_depot(*start);
    }
}

std::vector<Vector2i> InitialBuilder::bfs_to_closest(Vector2i start, bool(*f)(Vector2i)) {
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