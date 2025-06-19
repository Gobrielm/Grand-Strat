#pragma once

#include "company_ai.hpp"
#include "province.hpp"
#include <unordered_map>
#include <godot_cpp/classes/tile_map_layer.hpp>


using namespace godot;

class InitialBuilder : public CompanyAi {
    GDCLASS(InitialBuilder, CompanyAi);

    bool does_have_money_for_investment() override;
    void connect_road_depot(const Vector2i &depot);
    std::vector<Vector2i> bfs_to_closest(Vector2i start, bool(*f)(Vector2i));
    void build_factory_type(int type, Province* province);
    int get_levels_to_build(int type, Province* province) const;
    bool will_any_factory_be_cut_off(const Vector2i &fact_to_place) const;
    bool will_factory_by_cut_off(const Vector2i &factory_tile) const;
    int place_group_of_factories(const Vector2i &center);
    void build_and_connect_depots();
    void place_most_connected_depot(const Vector2i &center);
    int count_adjacent_factories(const Vector2i &center) const;

protected:
    static void _bind_methods();

public:
    
    static InitialBuilder* create(int p_country_id);

    InitialBuilder();
    InitialBuilder(int p_country_id);

    void build_initital_factories();
};

