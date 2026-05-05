#pragma once

#include <godot_cpp/classes/ref_counted.hpp>

using namespace godot;

class PDP: public RefCounted {
    GDCLASS(PDP, RefCounted);

    protected:

    static void _bind_methods();

    public:

    float price = 0;
    float supply = 0;
    float demand = 0;

    Dictionary buy_orders;
    Dictionary sell_orders;

    PDP() {}

    PDP(float p_price, float p_supply, float p_demand, Dictionary p_buy_orders, Dictionary p_sell_orders) {
        price = p_price;
        supply = p_supply;
        demand = p_demand;
        buy_orders = p_buy_orders;
        sell_orders = p_sell_orders;
    }

    PDP(float p_price, float p_supply, float p_demand, Array p_orders) {
        ERR_FAIL_COND_MSG(p_orders.size() != 2, "Orders is not size 2.");
        price = p_price;
        supply = p_supply;
        demand = p_demand;
        buy_orders = p_orders[0];
        sell_orders = p_orders[1];
    }

    // Getters
    float get_price() const;
    float get_supply() const;
    float get_demand() const;
    Dictionary get_buy_orders() const;
    Dictionary get_sell_orders() const;

    // Setters
    void set_price(float p_price);
    void set_supply(float p_supply);
    void set_demand(float p_demand);
    void set_buy_orders(Dictionary p_buy_orders);
    void set_sell_orders(Dictionary p_sell_orders);
};