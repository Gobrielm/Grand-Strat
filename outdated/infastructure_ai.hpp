#pragma once

#include "ai_base.hpp"
#include <unordered_map>
#include <godot_cpp/classes/tile_map_layer.hpp>

class RoadMap;
class TerminalMap;
class ProvinceManager;
class Factory;

using namespace godot;

class InfastructureAi : public AiBase {
    GDCLASS(InfastructureAi, AiBase);
    static constexpr int RAIL_THRESHOLD = 1000;
    static constexpr int ROAD_THRESHOLD = 20;

    //Utilities
    RoadMap* road_map;
    TileMapLayer* cargo_map;

    void check_for_unconnected_stations();
    int check_for_unconnected_buildings();
    int get_trade_weight(Vector2i tile);
    bool has_connected_station(Vector2i tile) const;
    
    std::vector<Vector2i> bfs_to_closest(Vector2i start, bool(*f)(Vector2i));

    void connect_factory(Ref<Factory> factory);

protected:
    static void _bind_methods();

public:
    static InfastructureAi* create(int p_country_id, int p_owner_id, Vector2i tile);

    InfastructureAi();
    InfastructureAi(int p_country_id, int p_owner_id, Vector2i tile);
    
    void build_roads();
    void build_road_depot();

    void connect_towns();
    void connect_factories();
};

