#include <queue>
#include "prospector_ai.hpp"
#include "factory_template.hpp"
#include "factory.hpp"
#include "town.hpp"
#include "../singletons/terminal_map.hpp"
#include "../singletons/road_map.hpp"
#include "../singletons/money_controller.hpp"
#include "../singletons/province_manager.hpp"
#include "../singletons/factory_creator.hpp"
#include "../singletons/recipe_info.hpp"
#include "../singletons/cargo_info.hpp"
#include "../singletons/pop_manager.hpp"


void ProspectorAi::_bind_methods() {
    ClassDB::bind_static_method(get_class_static(), D_METHOD("create", "p_country_id", "p_owner_id", "p_cargo_type"), &ProspectorAi::create);

    ClassDB::bind_method(D_METHOD("month_tick"), &ProspectorAi::month_tick);
}

ProspectorAi* ProspectorAi::create(int p_country_id, int p_owner_id, int p_cargo_type) {
    return memnew(ProspectorAi(p_country_id, p_owner_id, p_cargo_type));
}

ProspectorAi::ProspectorAi(): CompanyAi(), cargo_type(-1) {}
ProspectorAi::ProspectorAi(int p_country_id, int p_owner_id, int p_cargo_type): CompanyAi(p_country_id, p_owner_id), cargo_type(p_cargo_type) {}

void ProspectorAi::month_tick() {
    record_cash();
    pay_employees();
    if (does_have_money_for_investment()) {
        std::shared_ptr<Vector2i> town_tile = find_town_for_investment();
        if (town_tile != nullptr)
            build_building(*town_tile);
    }
}

void ProspectorAi::record_cash() {
    float cash = get_cash();
    {
        std::scoped_lock lock(m);
        if (past_cash.size() == MONTHS_OF_CASH_DATA)
            past_cash.pop_back();
        past_cash.push_front(cash);
    }
}

float ProspectorAi::get_cash() const {
    return MoneyController::get_instance()->get_money(get_owner_id());
}

float ProspectorAi::get_real_gross_profit(int months_to_average) const {
    ERR_FAIL_COND_V_EDMSG(months_to_average <= 0, 0, "Cannot average over a 0 or negitive amount of months");
    float total = 0;
    int i = 1;
    
    {
        std::scoped_lock lock(m);
        for (auto it = past_cash.begin(); it != past_cash.end(); it++) {
            total += (*it);
            if (++i > months_to_average) {
                break;
            }       
        }
    }
    
    return total / i;
}

void ProspectorAi::pay_employees() {
    float to_pay_each = get_wage(); // Profit over last n months averaged
    for (int pop_id: get_employees()) {
        PopManager::get_instance()->pay_pop(pop_id, to_pay_each);
    }
}


float ProspectorAi::get_wage() const {
    float income_wage = ((past_cash.back() - get_cash()) / past_cash.size()) * 0.9; // Profit over last n months averaged
    float total_wage_wanted = 0.0;
    const auto employees_copy = get_employees();
    for (const auto& id: employees_copy) {
        float exp_wage = PopManager::get_instance()->get_expected_wage(id);
        total_wage_wanted += exp_wage;
    }
    float ave_exp_wage = total_wage_wanted /= employees_copy.size();
    return std::max(income_wage, ave_exp_wage);
}

int ProspectorAi::get_cargo_type() const {
    return cargo_type;
}

bool ProspectorAi::needs_investment_from_pops() const {
    float profit = get_real_gross_profit(4) * 2.0f;
    return get_cash() < profit; // Less than 2 months of profit
}

bool ProspectorAi::does_have_money_for_investment() {
    float profit = get_real_gross_profit(4);
    return profit > 0;
}

std::shared_ptr<Vector2i> ProspectorAi::find_town_for_investment() {
    float highest_price = 0;
    std::shared_ptr<Vector2i> best_coords = nullptr;
    Ref<ProvinceManager> province_manager = ProvinceManager::get_instance();
    Ref<TerminalMap> terminal_map = TerminalMap::get_instance();
    std::unordered_set<int> prov_ids = province_manager->get_country_provinces(get_country_id());
    for (int prov_id: prov_ids) {
        Province* province = province_manager->get_province(prov_id);
        for (Vector2i tile: province->get_terminal_tiles_set()) {
            if (!terminal_map->is_town(tile)) continue;
            
            Ref<Town> town = terminal_map->get_town(tile);
            if (does_have_building_in_area_already(tile)) continue;

            float price = town->get_local_price(cargo_type);
            if (price > highest_price) {
                best_coords = std::make_shared<Vector2i>(tile);
                highest_price = price;
            }
            
        }
    }
    if (highest_price <= 0) return nullptr;
    //TODO: Potentially check if price will be profitable
    return best_coords;
}

bool ProspectorAi::does_have_building_in_area_already(const Vector2i& center) { // Will count non-maxed factories and construction sites
    Ref<TerminalMap> terminal_map = TerminalMap::get_instance();
    TileMapLayer* main_map = terminal_map -> get_main_map();
    const int MAX_TILES_OUT = 4;
    std::queue<Vector2i> q;
    std::unordered_map<Vector2i, int, godot_helpers::Vector2iHasher> tile_dist_map;
    q.push(center);
    tile_dist_map[center] = 0;
    Vector2i curr;
    while (q.size() != 0) {
        curr = q.front();
        q.pop();
        
        Array tiles = main_map->get_surrounding_cells(curr);
        for (int i = 0; i < tiles.size(); i++) {
            Vector2i tile = tiles[i];
            if (!terminal_map -> is_tile_traversable(tile, true) || !is_tile_owned(tile) || tile_dist_map.count(tile)) continue;
            if (tile_dist_map[curr] > MAX_TILES_OUT) continue;

            q.push(tile);
            tile_dist_map[tile] = tile_dist_map[curr] + 1;

            Ref<FactoryTemplate> factory = terminal_map -> get_terminal_as<FactoryTemplate>(tile);
            if (factory.is_valid() && factory->get_player_owner() == get_owner_id() && factory->get_outputs().count(cargo_type)) {
                
                if (!factory->is_max_level()) {
                    return true;
                }
                Ref<ConstructionSite> site = Ref<ConstructionSite>(factory);
                if (site.is_valid()) {
                    return true;
                }
            }
        }
    }
    return false;
}

void ProspectorAi::build_building(const Vector2i& town_tile) {
    auto tile_to_build_in = find_tile_for_new_building(town_tile);
    if (tile_to_build_in == nullptr) return;
    build_factory(*tile_to_build_in, town_tile);
}

std::shared_ptr<Vector2i> ProspectorAi::find_tile_for_new_building(const Vector2i& town_tile) {
    float best_weight = -1;
    std::shared_ptr<Vector2i> best_tile = nullptr;

    Ref<TerminalMap> terminal_map = TerminalMap::get_instance();
    TileMapLayer* main_map = terminal_map -> get_main_map();
    const int MAX_TILES_OUT = 10;
    std::queue<Vector2i> q;
    std::unordered_map<Vector2i, int, godot_helpers::Vector2iHasher> tile_dist_map;
    q.push(town_tile);
    tile_dist_map[town_tile] = 0;
    Vector2i curr;
    while (q.size() != 0) {
        curr = q.front();
        q.pop();
        
        Array tiles = main_map->get_surrounding_cells(curr);
        for (int i = 0; i < tiles.size(); i++) {
            Vector2i tile = tiles[i];
            if (!terminal_map -> is_tile_traversable(tile, true) || !is_tile_owned(tile) || tile_dist_map.count(tile)) continue;
            if (tile_dist_map[curr] > MAX_TILES_OUT) continue;

            q.push(tile);
            tile_dist_map[tile] = tile_dist_map[curr] + 1;
            if (!terminal_map -> is_tile_available(tile) || !is_factory_placement_valid(tile)) continue; //Tile Taken already

            float dist = tile.distance_to(town_tile);
            float score = get_build_score_for_factory(tile) + (30 / dist);
            if (score > best_weight) {
                best_tile = std::make_shared<Vector2i>(tile);
                best_weight = score;
            }
        }
    }
    return best_tile;
}

float ProspectorAi::get_build_score_for_factory(const Vector2i& tile) const {
    auto terminal_map = TerminalMap::get_instance();
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

void ProspectorAi::build_factory(const Vector2i& factory_tile, const Vector2i& town_tile) {
    int terminal_id = FactoryCreator::get_instance()->create_construction_site(factory_tile, get_owner_id());
    add_building(terminal_id);
    Recipe* recipe = RecipeInfo::get_instance()->get_primary_recipe_for_type(cargo_type);
    ERR_FAIL_COND_MSG(recipe == nullptr, "Recipe is null for type: " + CargoInfo::get_instance()->get_cargo_name(cargo_type));
    auto construction_site = TerminalMap::get_instance()->get_terminal_as<ConstructionSite>(terminal_id);
    construction_site -> set_recipe(recipe);
    connect_factory(factory_tile, town_tile);
}

void ProspectorAi::connect_factory(const Vector2i& factory_tile, const Vector2i& town_tile) {
    Ref<TerminalMap> terminal_map = TerminalMap::get_instance();
    TileMapLayer* main_map = terminal_map -> get_main_map();

    //IF depot exists for factory, or adj to town, no need to connect
    bool result = is_tile_adjacent(factory_tile, [town_tile] (const Vector2i& tile) { 
        return tile == town_tile || TerminalMap::get_instance()->is_road_depot(tile); 
    });
    if (result) return;

    auto town_depot = get_best_depot_tile_of_town(town_tile, factory_tile);
    if (!town_depot) {
        return;
    }

    auto fact_depot = get_best_depot_tile_of_factory(factory_tile, town_tile);
    if (!fact_depot) {
        return;
    }

    RoadMap::get_instance() -> bfs_and_connect(*town_depot, *fact_depot);
}

std::shared_ptr<Vector2i> ProspectorAi::get_best_depot_tile_of_town(const Vector2i& town_tile, const Vector2i& target) { //Will either get the closest depot, or build the closest
    Ref<TerminalMap> terminal_map = TerminalMap::get_instance();
    TileMapLayer* main_map = terminal_map -> get_main_map();
    //Placing or finding depot for town
    bool town_has_station = false;
    Vector2i best_tile;
    float closest = -1; //Prioritizes closest
    Array tiles = main_map->get_surrounding_cells(town_tile);
    for (int i = 0; i < tiles.size(); i++) {
        Vector2i tile = tiles[i];
        if (!is_tile_owned(tile)) continue;

        if (!terminal_map->is_tile_available(tile) && !terminal_map->is_road_depot(tile)) continue; // If blocked, by non road depot, cant


        float dist = tile.distance_to(target);
        if (dist < closest || closest == -1) {
            if (!town_has_station) { //No station yet
                closest = dist;
                best_tile = tile;
                town_has_station = terminal_map->is_road_depot(tile);
            } else if (town_has_station && terminal_map->is_road_depot(tile)) { //If found station that must only replace with only station
                closest = dist;
                best_tile = tile;
            }
        }
    }
    if (!town_has_station && best_tile != Vector2i(0, 0)) {
        place_depot(best_tile);
        //Check if bfs is needed
        if (is_tile_adjacent(best_tile, target)) return nullptr;
    } else if (best_tile == Vector2i(0, 0)) {
        print_error("No possible depots from town");
        return nullptr;
    }
    return std::make_shared<Vector2i>(best_tile); //Will return (0, 0) if no depot possible
}

std::shared_ptr<Vector2i> ProspectorAi::get_best_depot_tile_of_factory(const Vector2i& factory_tile, const Vector2i& target) {
    TileMapLayer* main_map = TerminalMap::get_instance()->get_main_map();
    float best_weight = -1;
    Vector2i best_tile;
    //Placing depot for factory
    Array tiles = main_map->get_surrounding_cells(factory_tile);
    for (int i = 0; i < tiles.size(); i++) {
        Vector2i tile = tiles[i];
        if (!TerminalMap::get_instance()->is_tile_available(tile)) continue;
        float weight = get_build_score_for_depot(tile, target);

        if ((weight > best_weight) || (best_weight == -1)) {
            best_weight = weight;
            best_tile = tile;
        }

        if (tile == target) return nullptr; //Town depot already borders both
    }

    if (best_weight == -1) {
        print_error("No possible depot location's for factory");
        return nullptr;
    }

    place_depot(best_tile);

    //Check to see if bfs is needed
    if (is_tile_adjacent(best_tile, target)) return nullptr; //Depot is adj to town, no bfs needed
    
    return std::make_shared<Vector2i>(best_tile);
}

float ProspectorAi::get_build_score_for_depot(const Vector2i& tile, const Vector2i& target) const {
    TileMapLayer* main_map = TerminalMap::get_instance()->get_main_map();
    float weight = 0;
    Array cells = main_map -> get_surrounding_cells(tile);
    for (int i = 0; i < cells.size(); i++) {
        Vector2i cell = cells[i];
        int val = TerminalMap::get_instance()->get_cargo_value_of_tile(cell, cargo_type);
        if (TerminalMap::get_instance()->is_tile_available(cell)) weight += val;
    }
    float dist = tile.distance_to(target);
    weight += (2 / dist);
    return weight;
}
