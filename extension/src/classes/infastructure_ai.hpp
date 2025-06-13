#pragma once

#include "ai_base.hpp"
#include <unordered_map>

using namespace godot;

enum AiActions {
    build_roads = 1,
    build_road_depot = 2,
    build_rails = 3,
    build_train_station = 4,
    nothing = 5
};

class InfastructureAi : public AiBase {
    GDCLASS(InfastructureAi, AiBase);
    static constexpr int RAIL_THRESHOLD = 1000;
    static constexpr int ROAD_THRESHOLD = 20;

    AiActions check_for_unconnected_stations();
    int check_for_unconnected_buildings();
    int get_trade_weight(Vector2i tile);
    bool is_tile_owned(Vector2i tile);
    bool has_connected_station(Vector2i tile) const;
    
    template<typename Predicate>
    std::vector<Vector2i> bfs_to_closest(Vector2i start, Predicate closest);


protected:
    static void _bind_methods();

public:
    static InfastructureAi* create(int p_country_id, int p_owner_id);

    InfastructureAi();
    InfastructureAi(int p_country_id, int p_owner_id);
    
    void build_roads();
    void build_rails();
    void build_road_depot();
    void build_train_station();

    AiActions decide_action();
    void month_tick();
    void run_until_nothing();
};

