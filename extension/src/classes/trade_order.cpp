#include "trade_order.hpp"
#include <godot_cpp/core/class_db.hpp>

using namespace godot;

void TradeOrder::_bind_methods() {
    ClassDB::bind_static_method(get_class_static(), D_METHOD("create", "p_type", "p_amount", "p_buy", "p_limit_price"), &TradeOrder::create);
    ClassDB::bind_static_method(get_class_static(), D_METHOD("create_buy_order", "p_type", "p_amount", "p_max_price"), &TradeOrder::create_buy_order);
    ClassDB::bind_static_method(get_class_static(), D_METHOD("create_sell_order", "p_type", "p_amount", "p_max_price"), &TradeOrder::create_sell_order);

    ClassDB::bind_method(D_METHOD("is_buy_order"), &TradeOrder::is_buy_order);
    ClassDB::bind_method(D_METHOD("is_sell_order"), &TradeOrder::is_sell_order);
    ClassDB::bind_method(D_METHOD("change_buy", "_buy"), &TradeOrder::change_buy);

    ClassDB::bind_method(D_METHOD("get_type"), &TradeOrder::get_type);
    ClassDB::bind_method(D_METHOD("change_amount", "p_amount"), &TradeOrder::change_amount);
    ClassDB::bind_method(D_METHOD("get_amount"), &TradeOrder::get_amount);

    ClassDB::bind_method(D_METHOD("get_limit_price"), &TradeOrder::get_limit_price);
    ClassDB::bind_method(D_METHOD("set_max_price", "p_max_price"), &TradeOrder::set_max_price);

    ClassDB::bind_method(D_METHOD("convert_to_array"), &TradeOrder::convert_to_array);
    ClassDB::bind_method(D_METHOD("price_is_acceptable", "price"), &TradeOrder::price_is_acceptable);

    ClassDB::bind_static_method("TradeOrder", D_METHOD("construct_from_array", "array"), &TradeOrder::construct_from_array);
}

TradeOrder::TradeOrder() {}

TradeOrder::TradeOrder(int p_type, int p_amount, bool p_buy, double p_limit_price) {
    initialize(p_type, p_amount, p_buy, p_limit_price);
}

TradeOrder* TradeOrder::create(int p_type, int p_amount, bool p_buy, double p_limit_price) {
    return memnew(TradeOrder(p_type, p_amount, p_buy, p_limit_price));
}

void TradeOrder::initialize(int p_type, int p_amount, bool p_buy, double p_limit_price) {
    type = p_type;
    amount = p_amount;
    buy = p_buy;
    max_price = p_limit_price;
}

TradeOrder* TradeOrder::create_buy_order(int p_type, int p_amount, double p_limit_price) {
    return memnew(TradeOrder(p_type, p_amount, true, p_limit_price));
}

TradeOrder* TradeOrder::create_sell_order(int p_type, int p_amount, double p_limit_price) {
    return memnew(TradeOrder(p_type, p_amount, false, p_limit_price));
}

bool TradeOrder::is_buy_order() const {
    return buy;
}

bool TradeOrder::is_sell_order() const {
    return !buy;
}

void TradeOrder::change_buy(bool _buy) {
    buy = _buy;
}

int TradeOrder::get_type() const {
    return type;
}

void TradeOrder::change_amount(int p_amount) {
    amount = p_amount;
}

int TradeOrder::get_amount() const {
    return amount;
}

double TradeOrder::get_limit_price() const {
    return max_price;
}

void TradeOrder::set_max_price(double p_limit_price) {
    max_price = p_limit_price;
}

Array TradeOrder::convert_to_array() const {
    Array arr;
    arr.push_back(type);
    arr.push_back(amount);
    arr.push_back(buy);
    arr.push_back(max_price);
    return arr;
}

bool TradeOrder::price_is_acceptable(double price) const {
    return buy ? price < max_price : price > max_price;
}

Ref<TradeOrder> TradeOrder::construct_from_array(const Array& array) {
    ERR_FAIL_COND_V(array.size() != 4, Ref<TradeOrder>());
    Ref<TradeOrder> order;
    order.instantiate();
    order->initialize(array[0], array[1], array[2], array[3]);
    return order;
}
