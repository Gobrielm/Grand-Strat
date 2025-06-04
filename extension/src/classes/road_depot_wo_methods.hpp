#pragma once

#include "broker.hpp"
#include "trade_order.hpp"
#include "local_price_controller.hpp"
#include "station.hpp"
#include "../singletons/road_map.hpp"
#include <queue>


#include <godot_cpp/variant/dictionary.hpp>
#include <godot_cpp/core/binder_common.hpp>
#include <godot_cpp/core/gdvirtual.gen.inc>

using namespace godot;

class RoadDepotWOMethods : public StationWOMethods {
    GDCLASS(RoadDepotWOMethods, StationWOMethods)

private:
    std::unordered_map<Vector2i, RoadDepotWOMethods*, godot_helpers::Vector2iHasher> other_road_depots = {};
    static constexpr int MAX_THROUGHPUT = 20;
    static constexpr int MAX_SUPPLY_DISTANCE = 10;
    int cargo_sent = 0;

    GDVIRTUAL0(supply_armies);

    virtual RoadDepotWOMethods* get_road_depot(Vector2i) const;
    GDVIRTUAL1RC(RoadDepotWOMethods*, get_road_depot, Vector2i);

protected:
    static void _bind_methods();

public:

    RoadDepotWOMethods();
    RoadDepotWOMethods(Vector2i new_location, int player_owner);
    ~RoadDepotWOMethods();

    virtual void initialize(Vector2i new_location, int player_owner) override;
    void distribute_cargo() override;
    void distribute_type(int type);
    void distribute_type_to_road_depot(int type, RoadDepotWOMethods* road_depot);

    void add_connected_road_depot(RoadDepotWOMethods* new_road_depot);
    void remove_connected_road_depot(const RoadDepotWOMethods* new_road_depot);

    void search_for_and_add_road_depots();

    virtual void day_tick();
};
