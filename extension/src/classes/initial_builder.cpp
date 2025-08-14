#include "initial_builder.hpp"
#include "town.hpp"
#include "ai_factory.hpp"
#include <algorithm>
#include <random>
#include <queue>
#include "../singletons/province_manager.hpp"
#include "../singletons/terminal_map.hpp"
#include "../singletons/cargo_info.hpp"
#include "../singletons/road_map.hpp"
#include "../singletons/factory_creator.hpp"
#include "../singletons/recipe_info.hpp"

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
    auto start_time = std::chrono::high_resolution_clock::now();
    Ref<ProvinceManager> province_manager = ProvinceManager::get_instance();
    Ref<TerminalMap> terminal_map = TerminalMap::get_instance();
    std::unordered_set<int> prov_ids = province_manager->get_country_provinces(get_country_id());
    for (const int &prov_id: prov_ids) {
        Province* province = province_manager->get_province(prov_id);
        if (province->has_town()) {
            build_factory_type(CargoInfo::get_instance()->get_cargo_type("grain"), province);
            build_factory_type(CargoInfo::get_instance()->get_cargo_type("wood"), province);
            build_factory_type(CargoInfo::get_instance()->get_cargo_type("salt"), province);
            build_t2_factory_in_towns(province);
        }
    }
    build_and_connect_depots();

    auto end_time = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end_time - start_time;
    print_line("Factory Placement took " + String::num_scientific(elapsed.count()) + " seconds");
}

void InitialBuilder::build_factory_type(int type, Province* province) {
    int tile_count = province->get_tiles_vector().size();
    int num_of_levels_to_place = get_levels_to_build(type, province);
    if (num_of_levels_to_place == 0) return;
    int levels_placed = 0;
    Ref<TerminalMap> terminal_map = TerminalMap::get_instance();
    for (const Vector2i &tile: province->get_town_centered_tiles()) {
        if (TerminalMap::get_instance()->is_tile_available(tile)) {
            int cargo_val = get_cargo_value_of_tile(tile, type);

            if (cargo_val != 0 && rand() % 3 == 0) {
                //Need to check if this factory will be cutoff, then check neighboors
                if (will_any_factory_be_cut_off(tile)) continue;
                int mult = std::min(rand() % cargo_val, cargo_val);
                FactoryCreator::get_instance()->create_primary_industry(type, tile, get_owner_id(), mult);

                levels_placed += mult;
                if (levels_placed > num_of_levels_to_place) {
                    break;
                }
            }
        }
    }
}

int InitialBuilder::get_levels_to_build(int type, Province* province) const {
    int num_pop = province->get_number_of_pops();
    String cargo_name = CargoInfo::get_instance()->get_cargo_name(type);
    std::unordered_map<int, float> cargo_needed = province->get_demand_for_needed_goods();
    ERR_FAIL_COND_V_MSG(!cargo_needed.count(type), 0, "No Demand for good of type: " + cargo_name);
    float demand = round(cargo_needed.at(type));
    if (cargo_name == "grain") {
        return get_levels_to_build_helper(type, demand);
    } else if (cargo_name == "wood") {
        return get_levels_to_build_helper(type, demand / 3 * 2);
    } else if (cargo_name == "salt") {
        return get_levels_to_build_helper(type, demand / 3 * 2);
    }
    return 0;
}

int InitialBuilder::get_levels_to_build_helper(int type, int demand) const {
    Recipe* recipe = RecipeInfo::get_instance()->get_primary_recipe_for_type_read_only(type);
    float ouput_quant = recipe->get_outputs()[type];
    int levels_to_build = round((demand) / (ouput_quant * 30));
    return levels_to_build;
}

bool InitialBuilder::will_any_factory_be_cut_off(const Vector2i &fact_to_place) const {
    Ref<TerminalMap> terminal_map = TerminalMap::get_instance();
    Array tiles = terminal_map->get_main_map()->get_surrounding_cells(fact_to_place);
    int available_tiles = 0; //Cehcks to see if fact to place can place
    for (int i = 0; i < tiles.size(); i++) {
        Vector2i tile = tiles[i];
        Ref<Broker> broker = terminal_map->get_broker(tile);
        if (broker.is_valid() && will_factory_by_cut_off(tile)) return true; // Checks factories that will be blocked
        if (terminal_map->is_tile_available(tile)) {
            available_tiles++;
        }
    }
    return available_tiles == 0;
}

bool InitialBuilder::will_factory_by_cut_off(const Vector2i &factory_tile) const { // Assuming one free tile will be taken
    Ref<TerminalMap> terminal_map = TerminalMap::get_instance();
    Array tiles = terminal_map->get_main_map()->get_surrounding_cells(factory_tile);
    int free_tiles = 0;
    for (int i = 0; i < tiles.size(); i++) {
        Vector2i tile = tiles[i];
        if (terminal_map->is_tile_available(tile)) {
            free_tiles++;
        }
    }
    return free_tiles <= 1;
}

void InitialBuilder::build_t2_factory_in_towns(Province* province) {
    Ref<TerminalMap> terminal_map = TerminalMap::get_instance();
    Ref<CargoInfo> cargo_info = CargoInfo::get_instance();
    for (const Vector2i &tile: province->get_town_tiles()) {
        Ref<Town> town = terminal_map->get_town(tile);
        build_t2_factory_in_town(town, cargo_info->get_cargo_type("lumber"));
        build_t2_factory_in_town(town, cargo_info->get_cargo_type("bread"));
    }
}

void InitialBuilder::build_t2_factory_in_town(Ref<Town> town, int output_type) {
    Recipe* recipe = RecipeInfo::get_instance()->get_secondary_recipe_for_type(output_type);
    ERR_FAIL_COND_MSG(recipe == nullptr, "Recipe is null from type: " + CargoInfo::get_instance()->get_cargo_name(output_type));
    Ref<AiFactory> factory = Ref<AiFactory>(memnew(AiFactory(town->get_location(), 0, recipe)));
    TerminalMap::get_instance()->create_isolated_terminal(factory);
    town->add_factory(factory);
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