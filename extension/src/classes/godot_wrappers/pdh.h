#pragma once

#include <godot_cpp/classes/ref_counted.hpp>

using namespace godot;

class PDH: public RefCounted {
    GDCLASS(PDH, RefCounted);

    protected:

    static void _bind_methods();

    public:

    float price = 0;
    int32_t supply = 0;
    int32_t demand = 0;

    Array sale_history;

    PDH() {}

    PDH(float p_price, int32_t p_supply, int32_t p_demand, Array p_sale_history) {
        price = p_price;
        supply = p_supply;
        demand = p_demand;
        sale_history = p_sale_history;
    }

    String to_string() {
        return String::num(price, 2) + " --- " + String::num_int64(supply) + " / " + String::num_int64(demand);
    }

    // Getters
    float get_price() const;
    int32_t get_supply() const;
    int32_t get_demand() const;
    Array get_sale_history() const;

    // Setters
    void set_price(float p_price);
    void set_supply(int32_t p_supply);
    void set_demand(int32_t p_demand);
    void set_sale_history(Array p_sale_history);
};