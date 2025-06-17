#pragma once

#include "company_ai.hpp"
#include <unordered_map>
#include <godot_cpp/classes/tile_map_layer.hpp>

class RoadMap;
class TerminalMap;
class ProvinceManager;
class Factory;

using namespace godot;

class ProspectorAi : public CompanyAi {
    GDCLASS(ProspectorAi, CompanyAi);
    int cargo_type; //Cargo the Company produces

    bool does_have_money_for_investment() override;
    Vector2i* find_town_for_investment();
    bool does_have_building_in_area_already(const Vector2i &center);
    Vector2i* find_tile_for_new_building(const Vector2i &town_tile);
    float get_build_score_for_factory(const Vector2i &tile) const;
    
    void build_factory(const Vector2i &factory_tile, const Vector2i &town_tile);
    void connect_factory(const Vector2i &factory_tile, const Vector2i &town_tile);
    Vector2i* get_best_depot_tile_of_town(const Vector2i &town_tile, const Vector2i &target);
    
    Vector2i* get_best_depot_tile_of_factory(const Vector2i &factory_tile, const Vector2i &target);
    float get_build_score_for_depot(const Vector2i &tile, const Vector2i &target) const;

protected:
    static void _bind_methods();

public:
    
    static ProspectorAi* create(int p_country_id, int p_owner_id, int p_cargo_type);

    ProspectorAi();
    ProspectorAi(int p_country_id, int p_owner_id, int p_cargo_type);

    void month_tick() override;
};

