#pragma once

#include "broker.hpp"
#include "trade_order.hpp"
#include "local_price_controller.hpp"
#include "terminal.hpp"
#include "../singletons/cargo_info.hpp"
#include <godot_cpp/variant/dictionary.hpp>

#include <godot_cpp/core/binder_common.hpp>
#include <godot_cpp/core/gdvirtual.gen.inc>

using namespace godot;

class StationWOMethods : public Broker {
    GDCLASS(StationWOMethods, Broker)

private:
    static constexpr int SUPPLY_DROPOFF = 1;
    static constexpr int MAX_SUPPLY_DISTANCE = 5;

    virtual void supply_armies();
    GDVIRTUAL0(supply_armies);
    void add_accepts(Broker* broker);
    void reset_accepts_train();

protected:
    static void _bind_methods();

public:
    static constexpr float SERVICE_FEE = 0.02;

    static Terminal* create(Vector2i new_location, int player_owner);

    StationWOMethods();
    StationWOMethods(Vector2i new_location, int player_owner);

    virtual void initialize(Vector2i new_location, int player_owner) override;

    float get_local_price(int type) const override;
    void place_order(int type, int amount, bool buy, float max_price) override;
    void edit_order(int type, int amount, bool buy, float max_price) override;
    int get_orders_magnitude();
    void remove_order(int type) override;
    virtual void distribute_cargo() override;

    virtual void add_connected_broker(Broker* new_broker) override;
    virtual void remove_connected_broker(const Broker* new_broker) override;

    virtual void refresh_accepts();

    virtual void day_tick();
};
