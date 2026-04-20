#pragma once

#include "ai_base.hpp"
#include <unordered_map>
#include <unordered_set>
#include <functional>
#include <godot_cpp/classes/tile_map_layer.hpp>

class RoadMap;
class TerminalMap;
class ProvinceManager;
class Factory;

using namespace godot;

class CompanyAi : public AiBase {
    GDCLASS(CompanyAi, AiBase);

protected:
    static void _bind_methods();

    std::unordered_set<int> employees;
    std::vector<int> exisiting_buildings; // terminal_ids
    std::unordered_set<int> get_employees() const;
    void add_building(int terminal_id);
    virtual bool does_have_money_for_investment();

    void place_depot(const Vector2i& tile);

    //Utilities
    RoadMap* road_map;
    TileMapLayer* cargo_map;
    Vector2i get_random_adjacent_tile(const Vector2i &center) const;
    int get_cargo_value_of_tile(const Vector2i &tile, int type) const;
    int get_cargo_value_of_tile(const Vector2i &tile, String cargo_name) const;
    std::vector<int> get_existing_buildings() const;

    bool is_factory_placement_valid(const Vector2i &fact_to_place) const;
    bool will_factory_by_cut_off(const Vector2i &factory_tile) const;

public:
    CompanyAi();
    CompanyAi(int p_country_id, Vector2i tile);
    CompanyAi(int p_country_id, int p_owner_id, Vector2i tile);

    //Utility Functions
    static bool is_tile_adjacent(const Vector2i &tile, const Vector2i &target);
    static bool is_tile_adjacent(const Vector2i &tile, const std::function<bool(const Vector2i&)>& f);
    static bool is_tile_adjacent_to_depot(const Vector2i &tile);

    void employ_pop(int pop_id);

    virtual void month_tick();
    virtual float get_wage() const;
};

