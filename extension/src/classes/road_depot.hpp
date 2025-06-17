#pragma once

#include <queue>
#include <set>

#include "firm.hpp"
#include <godot_cpp/variant/dictionary.hpp>

using namespace godot;

class RoadDepot : public Firm {
    GDCLASS(RoadDepot, Firm)

private:
    std::unordered_set<Vector2i, godot_helpers::Vector2iHasher> other_road_depots;
    static constexpr int MAX_THROUGHPUT = 20000;
    static constexpr float FEE = 0.05f;
    int cargo_sent = 0;
    float cash = 1000;

protected:
    std::unordered_set<Vector2i, godot_helpers::Vector2iHasher> connected_brokers;
    static constexpr int MAX_SUPPLY_DISTANCE = 10;

    static void _bind_methods();
    

public:
    static Ref<RoadDepot> create(const Vector2i new_location, const int player_owner);
    RoadDepot();
    RoadDepot(Vector2i new_location, int player_owner);
    virtual ~RoadDepot();

    void add_connected_broker(Ref<Broker> broker);
    void remove_connected_broker(const Ref<Broker> broker);

    void add_connected_road_depot(const Vector2i road_depot_tile);
    void remove_connected_road_depot(const Vector2i road_depot_tile);

    std::vector<Ref<Broker>> get_available_brokers_smart(int type, const Vector2i& source);
    bool is_tile_adjacent(Vector2i tile1, Vector2i tile2) const;
    std::vector<Ref<Broker>> get_available_brokers(int type);
    std::vector<Ref<Broker>> get_available_local_brokers(int type);
    std::vector<Ref<Broker>> get_available_local_brokers_smart(int type, const Vector2i& source);

    void refresh_other_road_depots();
    std::unordered_set<Vector2i, godot_helpers::Vector2iHasher> get_reachable_road_depots();

    static float get_fee();
    float get_cash() const override; //Temporary for testing
    void add_cash(float amount) override; //Temporary for testing
    void remove_cash(float amount) override; //Temporary for testing

    bool is_connected_to_other_depot() const;

    void month_tick();
};
