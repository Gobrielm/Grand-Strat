#pragma once

#include "firm.hpp"

#include <queue>
#include <unordered_map>
#include <unordered_set>

#include "../utility/vector2i_hash.hpp"
#include <godot_cpp/variant/vector2i.hpp>

class Broker;

using namespace godot;

class RoadDepot : public Firm {
    GDCLASS(RoadDepot, Firm)

private:
    std::unordered_set<Vector2i, godot_helpers::Vector2iHasher> other_road_depots;
    static constexpr int MAX_THROUGHPUT = 20000;
    static constexpr float FEE = 0.02f;
    int cargo_sent = 0;

protected:
    std::unordered_set<Vector2i, godot_helpers::Vector2iHasher> connected_brokers;

    static void _bind_methods();
    

public:
    static constexpr int MAX_SUPPLY_DISTANCE = 10;

    static Ref<RoadDepot> create(const Vector2i new_location, const int player_owner);
    RoadDepot();
    RoadDepot(Vector2i new_location, int player_owner);
    virtual ~RoadDepot();

    

    void add_connected_broker(Ref<Broker> broker);
    void remove_connected_broker(const Ref<Broker> broker);

    void add_connected_road_depot(const Vector2i& road_depot_tile);
    void remove_connected_road_depot(const Vector2i& road_depot_tile);

    bool is_connected_to_road_depot(const Vector2i& road_depot_tile) const;

    std::unordered_set<int> get_broker_ids_in_broad_market();
    std::unordered_set<int> get_broker_ids_in_local_market() const;
    
    std::vector<Ref<Broker>> get_available_brokers(int type);
    std::vector<Ref<Broker>> get_available_local_brokers(int type);

    void refresh_other_road_depots();
    std::unordered_set<Vector2i, godot_helpers::Vector2iHasher> get_reachable_road_depots();
    bool is_road_depot_valid(Ref<RoadDepot> road_depot) const;

    static float get_fee();

    bool is_connected_to_other_depot() const;

    void month_tick();
};
