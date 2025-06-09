#pragma once

#include "broker.hpp"
#include "trade_order.hpp"
#include "local_price_controller.hpp"
#include "station.hpp"
#include "../singletons/road_map.hpp"
#include "../singletons/cargo_info.hpp"
#include <queue>


#include <godot_cpp/variant/dictionary.hpp>
#include <godot_cpp/core/binder_common.hpp>
#include <godot_cpp/core/gdvirtual.gen.inc>

using namespace godot;

class RoadDepotWOMethods : public StationWOMethods {
    GDCLASS(RoadDepotWOMethods, StationWOMethods)

private:
    std::unordered_set<Vector2i, godot_helpers::Vector2iHasher> other_road_depots = {};
    static constexpr int MAX_THROUGHPUT = 20;
    static constexpr int MAX_SUPPLY_DISTANCE = 10;

    int cargo_sent = 0;
    float cash = 1000;

    GDVIRTUAL0(supply_armies);

protected:
    static void _bind_methods();

public:

    RoadDepotWOMethods();
    RoadDepotWOMethods(Vector2i new_location, int player_owner);
    virtual ~RoadDepotWOMethods();

    virtual void initialize(Vector2i new_location, int player_owner) override;
    void distribute_cargo() override;
    void distribute_type(int type);
    void distribute_type_to_road_depot(int type, Ref<RoadDepotWOMethods> road_depot);
    void distribute_type_to_broker(int type, Ref<Broker> broker);

    void add_connected_broker(Ref<Broker> broker) override;
    void remove_connected_broker(const Ref<Broker> broker) override;

    void add_connected_road_depot(Ref<RoadDepotWOMethods> new_road_depot);
    void remove_connected_road_depot(const Ref<RoadDepotWOMethods> new_road_depot);

    void add_accepts_from_depot(const Ref<RoadDepotWOMethods> road_depot);
    void refresh_accepts() override;
    virtual float get_local_price(int type) const;

    bool is_price_acceptable(int type, float pricePer) const override;
    bool is_price_acceptable_to_buy(int type, float pricePer) const;
    bool is_price_acceptable_to_sell(int type, float pricePer) const;

    void search_for_and_add_road_depots();

    virtual int get_desired_cargo(int type, float pricePer) const override;

    float get_cash() const override; //Temporary for testing
    void add_cash(float amount) override; //Temporary for testing
    void remove_cash(float amount) override; //Temporary for testing

    virtual void day_tick() override;
    virtual void month_tick();
};
