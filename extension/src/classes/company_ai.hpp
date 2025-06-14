#pragma once

#include "ai_base.hpp"
#include <unordered_map>
#include <godot_cpp/classes/tile_map_layer.hpp>

class RoadMap;
class TerminalMap;
class ProvinceManager;
class Factory;

using namespace godot;

class CompanyAi : public AiBase {
    GDCLASS(CompanyAi, AiBase);
    int cargo_type; //Cargo the Company produces

    //Utilities
    RoadMap* road_map;
    TileMapLayer* cargo_map;

    Vector2i* find_town_for_investment();
    Vector2i* find_tile_for_new_building(const Vector2i &town_tile);
    float get_build_score_for_tile(const Vector2i &tile) const;
    bool does_have_building_in_area_already(const Vector2i &center);
    bool does_have_money_for_investment();
    void build_factory(const Vector2i &factory_tile, const Vector2i &town_tile);
    void connect_factory(const Vector2i &factory_tile, const Vector2i &town_tile);

protected:
    static void _bind_methods();

public:
    static CompanyAi* create(int p_country_id, int p_owner_id, int p_cargo_type);

    CompanyAi();
    CompanyAi(int p_country_id, int p_owner_id, int p_cargo_type);

    void month_tick();
};

