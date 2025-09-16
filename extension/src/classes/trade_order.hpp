#pragma once

#include <godot_cpp/classes/object.hpp>
#include <godot_cpp/variant/array.hpp>

namespace godot {

class TradeOrder : public Object {
    GDCLASS(TradeOrder, Object);

private:
    int type = 0;
    int amount = 0;
    bool buy = true;
    double max_price = 0.0;

    void initialize(int p_type, int p_amount, bool p_buy, double p_limit_price);

protected:
    static void _bind_methods();

public:
    TradeOrder();
    TradeOrder(int p_type, int p_amount, bool p_buy, double p_limit_price);
    ~TradeOrder();

    static TradeOrder* create(int p_type, int p_amount, bool p_buy, double p_limit_price);

    static TradeOrder* create_buy_order(int p_type, int p_amount, double p_max_price);
    static TradeOrder* create_sell_order(int p_type, int p_amount, double p_max_price);

    bool is_buy_order() const;
    bool is_sell_order() const;
    void change_buy(bool _buy);

    int get_type() const;
    void change_amount(int p_amount);
    int get_amount() const;

    double get_limit_price() const;
    void set_max_price(double p_max_price);

    Array convert_to_array() const;
    bool is_price_acceptable(double price) const;

    static TradeOrder* construct_from_array(const Array& array);
};

}
