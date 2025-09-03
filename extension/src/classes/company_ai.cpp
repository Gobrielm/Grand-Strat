#include "company_ai.hpp"
#include "../singletons/terminal_map.hpp"
#include "../singletons/road_map.hpp"
#include "../singletons/money_controller.hpp"
#include "../singletons/province_manager.hpp"
#include "../singletons/cargo_info.hpp"
#include "../singletons/factory_creator.hpp"

void CompanyAi::_bind_methods() {}

CompanyAi::CompanyAi(): AiBase() {}
CompanyAi::CompanyAi(int p_country_id, int p_owner_id): AiBase(p_country_id, p_owner_id) {
    cargo_map = TerminalMap::get_instance() -> get_cargo_map();
    road_map = RoadMap::get_instance();
}

void CompanyAi::month_tick() {
    print_error("Did not implement month tick");
}

void CompanyAi::employ_pop(int pop_id) {
    std::scoped_lock lock(m);
    employees.insert(pop_id);
}

std::unordered_set<int> CompanyAi::get_employees() const {
    std::scoped_lock lock(m);
    return employees;
}

void CompanyAi::add_building(int terminal_id) {
    std::scoped_lock lock(m);
    exisiting_buildings.push_back(terminal_id);
}

bool CompanyAi::does_have_money_for_investment() {
    print_error("Did not implement does have money");
    return false;
}

void CompanyAi::place_depot(const Vector2i& tile) {
    RoadMap::get_instance()->place_road_depot(tile);
	int id = FactoryCreator::get_instance()->create_road_depot(tile, get_owner_id());
    add_building(id);
}

bool CompanyAi::is_tile_adjacent(const Vector2i &tile, const Vector2i &target) const {
    Array cells = TerminalMap::get_instance()->get_main_map() -> get_surrounding_cells(tile);
    for (int i = 0; i < cells.size(); i++) {
        Vector2i cell = cells[i];
        if (cell == target) return true;
    }
    return false;
}

bool CompanyAi::is_tile_adjacent(const Vector2i &tile, const std::function<bool(const Vector2i&)>& f) const {
    Array cells = TerminalMap::get_instance()->get_main_map() -> get_surrounding_cells(tile);
    for (int i = 0; i < cells.size(); i++) {
        Vector2i cell = cells[i];
        if (f(cell)) return true;
    }
    return false;
}

bool CompanyAi::is_tile_adjacent_to_depot(const Vector2i &tile) const {
    return is_tile_adjacent(tile, [](const Vector2i &tile){ 
        return TerminalMap::get_instance()->is_road_depot(tile); 
    });
}

Vector2i CompanyAi::get_random_adjacent_tile(const Vector2i &center) const {
    Array tiles = TerminalMap::get_instance()->get_main_map()->get_surrounding_cells(center);
    return tiles.pick_random();
}

int CompanyAi::get_cargo_value_of_tile(const Vector2i &tile, int type) const {
    return TerminalMap::get_instance()->get_cargo_value_of_tile(tile, type);
}

int CompanyAi::get_cargo_value_of_tile(const Vector2i &tile, String cargo_name) const {
    return get_cargo_value_of_tile(tile, CargoInfo::get_instance()->get_cargo_type(cargo_name));
}

std::vector<int> CompanyAi::get_existing_buildings() const {
    std::scoped_lock lock(m);
    return exisiting_buildings;
}

bool CompanyAi::is_factory_placement_valid(const Vector2i &fact_to_place) const {
    Ref<TerminalMap> terminal_map = TerminalMap::get_instance();
    Array tiles = terminal_map->get_main_map()->get_surrounding_cells(fact_to_place);
    int available_tiles = 0; //Checks to see if fact to place can place
    for (int i = 0; i < tiles.size(); i++) {
        Vector2i tile = tiles[i];
        Ref<Broker> broker = terminal_map->get_broker(tile);
        if (broker.is_valid() && will_factory_by_cut_off(tile)) return false; // Checks factories that will be blocked
        if (terminal_map->is_tile_available(tile)) {
            available_tiles++;
        }
    }
    return available_tiles > 0;
}

bool CompanyAi::will_factory_by_cut_off(const Vector2i &factory_tile) const {
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

float CompanyAi::get_wage() const {
    return 0;
}