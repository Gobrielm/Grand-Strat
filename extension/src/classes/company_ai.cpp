#include "company_ai.hpp"
#include "../singletons/terminal_map.hpp"
#include "../singletons/road_map.hpp"
#include "../singletons/money_controller.hpp"
#include "../singletons/province_manager.hpp"

void CompanyAi::_bind_methods() {
    ClassDB::bind_static_method(get_class_static(), D_METHOD("create", "p_country_id", "p_owner_id", "p_cargo_type"), &CompanyAi::create);

    ClassDB::bind_method(D_METHOD("month_tick"), &CompanyAi::month_tick);
}

CompanyAi* CompanyAi::create(int p_country_id, int p_owner_id, int p_cargo_type) {
    return memnew(CompanyAi(p_country_id, p_owner_id, p_cargo_type));
}

CompanyAi::CompanyAi(): AiBase() {}
CompanyAi::CompanyAi(int p_country_id, int p_owner_id, int p_cargo_type): AiBase(p_country_id, p_owner_id), cargo_type(p_cargo_type) {
    cargo_map = TerminalMap::get_instance() -> get_cargo_map();
    road_map = RoadMap::get_instance();
}

void CompanyAi::month_tick() {
    if (does_have_money_for_investment()) {
        print_line("has money");
        Vector2i* town_tile = find_town_for_investment();
        if (town_tile == nullptr) return; 
        print_line("found town");
        Vector2i town_to_expand_from = *town_tile;
        
        if (!does_have_building_in_area_already(town_to_expand_from)) {
            print_line("no town building");
            Vector2i* tile_to_build_in = find_tile_for_new_building(town_to_expand_from);
            if (tile_to_build_in == nullptr) return;
            print_line("found place to build");
            build_factory(*tile_to_build_in, *town_tile);
            print_line("built");
            memdelete(tile_to_build_in);
        }
        memdelete(town_tile);
    }
}

bool CompanyAi::does_have_money_for_investment() {
    float cash = MoneyController::get_instance()->get_money(get_owner_id());
    return cash > FactoryTemplate::get_cost_for_upgrade() * 3.5;
}

Vector2i* CompanyAi::find_town_for_investment() {
    float max_mult = 0;
    Vector2i best_coords;
    Ref<ProvinceManager> province_manager = ProvinceManager::get_instance();
    Ref<TerminalMap> terminal_map = TerminalMap::get_instance();
    std::unordered_set<int> prov_ids = province_manager->get_country_provinces(get_country_id());
    for (int prov_id: prov_ids) {
        Province* province = province_manager->get_province(prov_id);
        for (Vector2i tile: province->get_terminal_tiles_set()) {
            if (terminal_map->is_town(tile)) {
                Ref<Town> town = terminal_map->get_town(tile);
                float price_mult = town->get_local_price(cargo_type) / LocalPriceController::get_base_price(cargo_type);
                if (price_mult > max_mult) {
                    best_coords = tile;
                    max_mult = price_mult;
                }
            }
        }
    }
    if (max_mult < 1) {
        return nullptr;
    }
    return memnew(Vector2i(best_coords));
}

bool CompanyAi::does_have_building_in_area_already(const Vector2i &center) {
    Ref<TerminalMap> terminal_map = TerminalMap::get_instance();
    TileMapLayer* main_map = terminal_map -> get_main_map();
    const int MAX_TILES_OUT = 5;
    std::queue<Vector2i> q;
    std::unordered_map<Vector2i, int, godot_helpers::Vector2iHasher> m;
    q.push(center);
    m[center] = 0;
    Vector2i curr;
    while (q.size() != 0) {
        curr = q.front();
        q.pop();
        
        Array tiles = main_map->get_surrounding_cells(curr);
        for (int i = 0; i < tiles.size(); i++) {
            Vector2i tile = tiles[i];
            if (!terminal_map -> is_tile_traversable(tile, true) || !is_tile_owned(tile) || m.count(tile)) continue;
            if (m[curr] > MAX_TILES_OUT) continue;

            q.push(tile);
            m[tile] = m[curr] + 1;

            Ref<Factory> factory = terminal_map -> get_terminal_as<Factory>(tile);
            if (factory.is_valid() && factory->get_player_owner() == get_owner_id() && factory->outputs.count(cargo_type) && !factory->is_max_level()) {
                return true;
            }
        }
    }
    return false;
}

Vector2i* CompanyAi::find_tile_for_new_building(const Vector2i &town_tile) {
    float best_weight = -1;
    Vector2i best_tile;

    Ref<TerminalMap> terminal_map = TerminalMap::get_instance();
    TileMapLayer* main_map = terminal_map -> get_main_map();
    const int MAX_TILES_OUT = 10;
    std::queue<Vector2i> q;
    std::unordered_map<Vector2i, int, godot_helpers::Vector2iHasher> m;
    q.push(town_tile);
    m[town_tile] = 0;
    Vector2i curr;
    while (q.size() != 0) {
        curr = q.front();
        q.pop();
        
        Array tiles = main_map->get_surrounding_cells(curr);
        for (int i = 0; i < tiles.size(); i++) {
            Vector2i tile = tiles[i];
            if (!terminal_map -> is_tile_traversable(tile, true) || !is_tile_owned(tile) || m.count(tile)) continue;
            if (m[curr] > MAX_TILES_OUT) continue;

            q.push(tile);
            m[tile] = m[curr] + 1;
            if (!terminal_map -> is_tile_available(tile)) continue; //Tile Taken already

            float dist = tile.distance_to(town_tile);
            float score = get_build_score_for_tile(tile) + (50 / dist);
            if (score > best_weight) {
                best_tile = tile;
                best_weight = score;
            }
        }
    }
    if (best_weight > 0) {
        return memnew(Vector2i(best_tile));
    }
    return nullptr;
}

float CompanyAi::get_build_score_for_tile(const Vector2i &tile) const {
    Ref<TerminalMap> terminal_map = TerminalMap::get_instance();
    TileMapLayer* main_map = terminal_map -> get_main_map();
    float score = terminal_map->get_cargo_value_of_tile(tile, cargo_type);
    if (score == 0.0) return score; 
    Array tiles = main_map->get_surrounding_cells(tile);
    for (int i = 0; i < tiles.size(); i++) {
        Vector2i cell = tiles[i];
        Ref<Factory> factory = terminal_map->get_terminal_as<Factory>(cell);
        if (!terminal_map -> is_tile_traversable(tile, true) || !is_tile_owned(tile) || (factory.is_valid() && factory->get_player_owner() != get_owner_id())) continue;
        score += terminal_map->get_cargo_value_of_tile(cell, cargo_type) / 2;
        if (factory.is_valid()) score += terminal_map->get_cargo_value_of_tile(cell, cargo_type) / 2;
        if (terminal_map->is_road_depot(cell)) score *= 2;
        if (terminal_map->is_town(cell)) return -100000; //Debug, do not place adjacent to towns 
    }
    return score;
}

void CompanyAi::build_factory(const Vector2i &factory_tile, const Vector2i &town_tile) {
    cargo_map->call("create_construction_site", get_owner_id(), factory_tile);
    Array a;
    a.push_back(Dictionary());
    Dictionary d;
    d[cargo_type] = 1;
    a.push_back(d);
    TerminalMap::get_instance() -> set_construction_site_recipe(factory_tile, a);
    connect_factory(factory_tile, town_tile);
}

void CompanyAi::connect_factory(const Vector2i &factory_tile, const Vector2i &town_tile) {
    Ref<TerminalMap> terminal_map = TerminalMap::get_instance();
    TileMapLayer* main_map = terminal_map -> get_main_map();
    int type = cargo_type;
    auto get_tile_weight = [main_map, town_tile, type](Vector2i tile) -> float {
        float weight = 0;
        Array cells = main_map -> get_surrounding_cells(tile);
        for (int i = 0; i < cells.size(); i++) {
            Vector2i cell = cells[i];
            int val = TerminalMap::get_instance()->get_cargo_value_of_tile(cell, type);
            if (TerminalMap::get_instance()->is_tile_available(cell)) weight += val;
        }
        float dist = tile.distance_to(town_tile);
        weight += (50 / dist);
        return weight;
    };

    //IF depot exists for factory, no need to connect
    Array tiles = main_map->get_surrounding_cells(factory_tile);
    for (int i = 0; i < tiles.size(); i++) {
        Vector2i tile = tiles[i];
        if (tile == town_tile) return; //Already adjacent, no need to connect
        if (terminal_map->is_road_depot(tile)) return; //Potentially check if it actually connects to town
    }

    //Placing or finding depot for town
    bool town_has_station = false;
    Vector2i target;
    float closest = -1; //Prioritizes closest
    tiles = main_map->get_surrounding_cells(town_tile);
    for (int i = 0; i < tiles.size(); i++) {
        Vector2i tile = tiles[i];
        float dist = tile.distance_to(factory_tile);
        if ((dist < closest || closest == -1) && !town_has_station) {
            if (!terminal_map->is_tile_available(tile) && !terminal_map->is_road_depot(tile)) {
                continue; // Tile taken and isn't taken by road depot
            }
            if (!town_has_station) { //No station yet
                closest = dist;
                target = tile;
                town_has_station = terminal_map->is_road_depot(tile);
            } else if (town_has_station && terminal_map->is_road_depot(tile)) { //If found station that must only replace with only station
                closest = dist;
                target = tile;
            }
        }

    }
    if (!town_has_station) {
        if (target == Vector2i(0, 0)) {
            //No available stops, should try to connect to other town eventually
            return;
        }
        place_depot(target);
    }


    float best_weight = -1;
    Vector2i best_tile;
    //Placing depot for factory
    tiles = main_map->get_surrounding_cells(factory_tile);
    for (int i = 0; i < tiles.size(); i++) {
        Vector2i tile = tiles[i];
        if (!terminal_map->is_tile_available(tile)) continue;
        float weight = get_tile_weight(tile);

        if ((weight > best_weight) || (best_weight == -1)) {
            best_weight = weight;
            best_tile = tile;
        }

        if (tile == target) return; //Town depot already borders both
    }

    if (best_weight == -1) return;

    tiles = main_map->get_surrounding_cells(best_tile);
    for (int i = 0; i < tiles.size(); i++) {
        Vector2i tile = tiles[i];
        if (tile == town_tile) return; //Depot is adj to town, no bfs needed
    }

    place_depot(best_tile);

    RoadMap::get_instance() -> bfs_and_connect(target, best_tile);
}

