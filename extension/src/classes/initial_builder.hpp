#pragma once

#include "company_ai.hpp"
#include "province.hpp"
#include <unordered_map>
#include <godot_cpp/classes/object.hpp>
#include <godot_cpp/classes/tile_map_layer.hpp>


using namespace godot;

class InitialBuilder: public Object {
    GDCLASS(InitialBuilder, Object);
    int country_id;
    int owner_id;

    std::unordered_map<Vector2i, int, godot_helpers::Vector2iHasher> factories_to_place_on_map; // Tile -> type, if type is negitive, place road_depot

    void connect_road_depot(const Vector2i &depot);
    std::vector<Vector2i> bfs_to_closest(Vector2i start, bool(*f)(Vector2i));
    void build_factory_type(int type, Province* province);
    bool is_factory_placement_valid(const Vector2i &fact_to_place) const;
    bool will_factory_by_cut_off(const Vector2i &factory_tile) const;
    int get_levels_to_build(int type, Province* province) const;
    int get_levels_to_build_helper(int type, int demand) const;
    void build_t2_factory_in_towns(Province* province);
    void build_t2_factory_in_town(Ref<Town> town, int output_type);
    void build_and_connect_depots();
    void place_most_connected_depot(const Vector2i &center);
    int count_adjacent_factories(const Vector2i &center) const;
    void place_depot(const Vector2i& tile);
    bool is_tile_owned(Vector2i tile) const;

protected:
    static void _bind_methods();

public:
    
    static InitialBuilder* create(int p_country_id);

    InitialBuilder();
    InitialBuilder(int p_country_id);

    void build_initital_factories();
};

