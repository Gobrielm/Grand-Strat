#pragma once

#include <godot_cpp/classes/ref_counted.hpp>

using namespace godot;

class PDP: public RefCounted {
    GDCLASS(PDP, RefCounted);

    protected:

    static void _bind_methods();

    public:

    float price = 0;
    int32_t supply = 0;
    int32_t demand = 0;

    Array buy_orders;
    Array sell_orders;

    PDP() {}

    PDP(float p_price, int32_t p_supply, int32_t p_demand, Array p_buy_orders, Array p_sell_orders) {
        price = p_price;
        supply = p_supply;
        demand = p_demand;
        buy_orders = p_buy_orders;
        sell_orders = p_sell_orders;
    }

    PDP(float p_price, int32_t p_supply, int32_t p_demand, Array p_orders) {
        price = p_price;
        supply = p_supply;
        demand = p_demand;
        if (p_orders.size() == 2) {
            buy_orders = p_orders[0];
            sell_orders = p_orders[1];
        }
        
    }

    String to_string() {
        return String::num(price, 2) + " --- " + String::num_int64(supply) + " / " + String::num_int64(demand);
    }

    // Getters
    float get_price() const;
    int32_t get_supply() const;
    int32_t get_demand() const;
    Array get_buy_orders() const;
    Array get_sell_orders() const;

    // Setters
    void set_price(float p_price);
    void set_supply(int32_t p_supply);
    void set_demand(int32_t p_demand);
    void set_buy_orders(Array p_buy_orders);
    void set_sell_orders(Array p_sell_orders);
};