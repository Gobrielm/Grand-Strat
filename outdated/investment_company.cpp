#include <queue>
#include "investment_company.hpp"
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

void InvestmentCompany::_bind_methods() {
    ClassDB::bind_static_method(get_class_static(), D_METHOD("create", "p_country_id", "tile", "p_cargo_type"), &InvestmentCompany::godot_create);

    ClassDB::bind_method(D_METHOD("month_tick"), &InvestmentCompany::month_tick);
}

Ref<InvestmentCompany> InvestmentCompany::godot_create(int p_country_id, Vector2i tile, int p_cargo_type) {
    return create(p_country_id, tile, p_cargo_type);
}

Ref<InvestmentCompany> InvestmentCompany::create(int p_country_id, Vector2i tile, int p_cargo_type) {
    return memnew(InvestmentCompany(p_country_id, tile, p_cargo_type));
}

Ref<InvestmentCompany> InvestmentCompany::create(int p_country_id, int p_owner_id, Vector2i tile, int p_cargo_type) {
    return memnew(InvestmentCompany(p_country_id, p_owner_id, tile, p_cargo_type));
}

InvestmentCompany::InvestmentCompany(): CompanyAi(), cargo_type(-1) {}
InvestmentCompany::InvestmentCompany(int p_country_id, Vector2i tile, int p_cargo_type): CompanyAi(p_country_id, tile), cargo_type(p_cargo_type) {}
InvestmentCompany::InvestmentCompany(int p_country_id, int p_owner_id, Vector2i tile, int p_cargo_type): CompanyAi(p_country_id, p_owner_id, tile), cargo_type(p_cargo_type) {}

void InvestmentCompany::month_tick() {
    record_cash();
    pay_employees();
    if (does_have_money_for_investment() && should_build()) {
        std::shared_ptr<Vector2i> town_tile = find_town_for_investment();
        if (town_tile != nullptr) {
            build_building(*town_tile);
        }
    }
}

void InvestmentCompany::record_cash() {
    float cash = get_cash();
    {
        std::scoped_lock lock(m);
        if (past_cash.size() == MONTHS_OF_CASH_DATA)
            past_cash.pop_back();
        past_cash.push_front(cash);
    }
}

float InvestmentCompany::get_net_profit(int months_to_average) const {
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

void InvestmentCompany::pay_employees() {
    float to_pay_each = get_wage(); // Profit over last n months averaged
    for (int pop_id: get_employees()) {
        PopManager::get_instance()->pay_pop(pop_id, to_pay_each);
    }
}


float InvestmentCompany::get_wage() const {
    const auto employees_copy = get_employees();
    const auto& num_of_employees = std::max(1, int(employees_copy.size()));
    float income_wage = get_net_profit(2) * 0.9 / num_of_employees; // Most of gross profit
    float total_wage_wanted = 0.0;
    for (const auto& id: employees_copy) {
        float exp_wage = PopManager::get_instance()->get_expected_wage(id);
        total_wage_wanted += exp_wage;
    }
    float ave_exp_wage = total_wage_wanted / num_of_employees;
    return std::max(income_wage, ave_exp_wage);
}

int InvestmentCompany::get_cargo_type() const {
    return cargo_type;
}

bool InvestmentCompany::is_seeking_investment_from_pops() const {
    float two_months_of_profit = get_net_profit(4) * 2.0f;

    int employees_size = get_employees().size();
    // Won't hire an excessive amount, still lenient
    float building_ratio = employees_size != 0 ? float(get_existing_buildings().size() + 2) / employees_size: 10;
    
    return get_cash() < two_months_of_profit && building_ratio >= 1; // Less than 2 months of profit
}

bool InvestmentCompany::does_have_money_for_investment() {
    float profit = get_net_profit(4);
    return profit > 0;
}

bool InvestmentCompany::should_build() const {
    auto terminal_map = TerminalMap::get_instance();
    int count = 0;
    auto existing_buildings_copy = get_existing_buildings();

    for (const auto& building_id: existing_buildings_copy) {
        auto fact = terminal_map->get_terminal_as<FactoryTemplate>(building_id);
        if (fact->is_constructing()) count++;
    }
    return (float(count) / existing_buildings_copy.size()) < 0.15; // Isn't building too much
}

std::shared_ptr<Vector2i> InvestmentCompany::find_town_for_investment() {
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

bool InvestmentCompany::does_have_building_in_area_already(const Vector2i& center) { // Will count non-maxed factories and construction sites
    Ref<TerminalMap> terminal_map = TerminalMap::get_instance();
    TileMapLayer* main_map = terminal_map -> get_main_map();
    const int MAX_TILES_OUT = 4;
    std::queue<Vector2i> q;
    std::unordered_map<Vector2i, int, godot_helpers::Vector2iHasher> tile_dist_map;
    int number_of_competitor_construct_sites = 0;
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
            if (factory.is_valid() && factory->get_outputs().count(cargo_type)) {
                
                if (!factory->is_max_level() && factory->get_player_owner() == get_owner_id()) {
                    return true;
                }
                Ref<ConstructionSite> site = Ref<ConstructionSite>(factory);
                if (site.is_valid()) {
                    if (factory->get_player_owner() == get_owner_id()) {
                        return true;
                    } else {
                        number_of_competitor_construct_sites++;
                    }
                    
                }
            }
        }
    }
    return number_of_competitor_construct_sites >= 3;
}

void InvestmentCompany::build_building(const Vector2i& town_tile) {
    auto tile_to_build_in = find_tile_for_new_building(town_tile);
    if (tile_to_build_in == nullptr) return;
    build_factory(*tile_to_build_in, town_tile);
}

std::shared_ptr<Vector2i> InvestmentCompany::find_tile_for_new_building(const Vector2i& town_tile) {
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

float InvestmentCompany::get_build_score_for_factory(const Vector2i& tile) const {
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

void InvestmentCompany::build_factory(const Vector2i& factory_tile, const Vector2i& town_tile) {
    bool is_in_town = town_tile == factory_tile;
    int terminal_id = 0;
    if (is_in_town) {
        Ref<ConstructionSite> construction_site = memnew(ConstructionSite(factory_tile, get_owner_id()));
        terminal_id = construction_site->get_terminal_id();
        TerminalMap::get_instance()->create_isolated_factory_in_town(construction_site);
    } else {
        terminal_id = FactoryCreator::get_instance()->create_construction_site(factory_tile, get_owner_id());
    }
    
    add_building(terminal_id);
    Recipe* recipe = RecipeInfo::get_instance()->get_recipe_for_type(cargo_type);
    ERR_FAIL_COND_MSG(recipe == nullptr, "Recipe is null for type: " + CargoInfo::get_instance()->get_cargo_name(cargo_type));
    auto construction_site = TerminalMap::get_instance()->get_terminal_as<ConstructionSite>(terminal_id);
    print_line("Built building at " + String(factory_tile));
    construction_site -> set_recipe(recipe);
    connect_factory(factory_tile, town_tile);
}

void InvestmentCompany::build_factory_instantly(const Vector2i& factory_tile, const Vector2i& town_tile) {
    Recipe* recipe = RecipeInfo::get_instance()->get_recipe_for_type(cargo_type);
    ERR_FAIL_COND_MSG(recipe == nullptr, "Recipe is null for type: " + CargoInfo::get_instance()->get_cargo_name(cargo_type));
    auto terminal_map = TerminalMap::get_instance();
    bool is_in_town = town_tile == factory_tile;
    Ref<Factory> factory = memnew(Factory(factory_tile, get_owner_id(), recipe));
    if (is_in_town) {
        terminal_map->create_isolated_factory_in_town(factory);
    } else {
        terminal_map->encode_factory(factory);
    }

    add_building(factory->terminal_id);
    print_line("Built building at " + String(factory_tile));
    connect_factory(factory_tile, town_tile);
}

void InvestmentCompany::connect_factory(const Vector2i& factory_tile, const Vector2i& town_tile) {
    Ref<TerminalMap> terminal_map = TerminalMap::get_instance();
    TileMapLayer* main_map = terminal_map -> get_main_map();

    //IF depot exists for factory, or adj to town, no need to connect
    bool result = (factory_tile == town_tile) || is_tile_adjacent(factory_tile, [town_tile] (const Vector2i& tile) { 
        return tile == town_tile; 
    });
    if (result) return;

    auto town_depot = get_best_depot_tile_or_place_best(town_tile, factory_tile);
    if (!town_depot) {
        return;
    }

    auto fact_depot = get_best_depot_tile_or_place_best(factory_tile, town_tile);
    if (!fact_depot) {
        return;
    }

    auto depot1 = terminal_map->get_terminal_as<RoadDepot>(*town_depot);
    if (depot1->is_connected_to_road_depot(*fact_depot)) return; // Chosen depots already connected

    RoadMap::get_instance() -> bfs_and_connect(*town_depot, *fact_depot);
}

std::shared_ptr<Vector2i> InvestmentCompany::get_best_depot_tile_or_place_best(const Vector2i& center_tile, const Vector2i& target) {
    Ref<TerminalMap> terminal_map = TerminalMap::get_instance();
    TileMapLayer* main_map = terminal_map -> get_main_map();
    //Placing or finding depot for town
    bool has_depot = false;
    Vector2i best_tile;
    float closest = 100; //Prioritizes closest
    Array tiles = main_map->get_surrounding_cells(center_tile);
    for (int i = 0; i < tiles.size(); i++) {
        Vector2i tile = tiles[i];
        if (!is_tile_owned(tile) || !terminal_map->is_tile_traversable(tile, true)) continue;
        
        float dist = tile.distance_to(target);
        bool will_replace = (dist < closest || best_tile == Vector2i(0, 0));

        if (terminal_map->is_road_depot(tile)) {
            if (!has_depot || (has_depot && will_replace)) {
                has_depot = true;
                closest = dist;
                best_tile = tile;
            }
        } else if (!has_depot && terminal_map->is_tile_available(tile) && will_replace) {
            closest = dist;
            best_tile = tile;
        }
    }
    ERR_FAIL_COND_V_MSG(best_tile == Vector2i(0, 0), nullptr, "No available locations for depot.");

    if (!has_depot) {
        place_depot(best_tile);
    }

    return std::make_shared<Vector2i>(best_tile); //Will return (0, 0) if no depot possible
}
float InvestmentCompany::get_build_score_for_depot(const Vector2i& tile, const Vector2i& target) const {
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
