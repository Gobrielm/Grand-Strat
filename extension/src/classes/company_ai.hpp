#pragma once

#include "ai_base.hpp"
#include <unordered_map>
#include <unordered_set>
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
    virtual bool does_have_money_for_investment();

    //Utilities
    RoadMap* road_map;
    TileMapLayer* cargo_map;
    //Utility Functions
    bool is_tile_adjacent(const Vector2i &tile, const Vector2i &target) const;
    bool is_tile_adjacent(const Vector2i &tile, bool(*f)(const Vector2i&)) const;
    bool is_tile_adjacent_to_depot(const Vector2i &tile) const;
    Vector2i get_random_adjacent_tile(const Vector2i &center) const;
    int get_cargo_value_of_tile(const Vector2i &tile, int type) const;
    int get_cargo_value_of_tile(const Vector2i &tile, String cargo_name) const;

public:
    CompanyAi();
    CompanyAi(int p_country_id, int p_owner_id);

    void employ_pop(int pop_id);

    virtual void month_tick();
    virtual float get_wage() const;
};

