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
    std::unordered_map<Vector2i, RoadDepotWOMethods*, godot_helpers::Vector2iHasher> other_road_depots = {};
    static constexpr int MAX_THROUGHPUT = 20;
    static constexpr int MAX_SUPPLY_DISTANCE = 10;

    int cargo_sent = 0;
    float cash = 1000;

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
    void distribute_type_to_broker(int type, Broker* broker);

    void add_connected_broker(Broker* broker) override;
    void remove_connected_broker(const Broker* broker) override;

    void add_connected_road_depot(RoadDepotWOMethods* new_road_depot);
    void remove_connected_road_depot(const RoadDepotWOMethods* new_road_depot);

    void add_accepts_from_depot(const RoadDepotWOMethods* road_depot);
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
