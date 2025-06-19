#include "company_ai.hpp"
#include "../singletons/terminal_map.hpp"
#include "../singletons/road_map.hpp"
#include "../singletons/money_controller.hpp"
#include "../singletons/province_manager.hpp"
#include "../singletons/cargo_info.hpp"

void CompanyAi::_bind_methods() {}

CompanyAi::CompanyAi(): AiBase() {}
CompanyAi::CompanyAi(int p_country_id, int p_owner_id): AiBase(p_country_id, p_owner_id) {
    cargo_map = TerminalMap::get_instance() -> get_cargo_map();
    road_map = RoadMap::get_instance();
}

void CompanyAi::month_tick() {
    print_error("Did not implement month tick");
}

bool CompanyAi::does_have_money_for_investment() {
    print_error("Did not implement does have money");
    return false;
}

bool CompanyAi::is_tile_adjacent(const Vector2i &tile, const Vector2i &target) const {
    Array cells = TerminalMap::get_instance()->get_main_map() -> get_surrounding_cells(tile);
    for (int i = 0; i < cells.size(); i++) {
        Vector2i cell = cells[i];
        if (cell == target) return true;
    }
    return false;
}

bool CompanyAi::is_tile_adjacent(const Vector2i &tile, bool(*f)(const Vector2i&)) const {
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
