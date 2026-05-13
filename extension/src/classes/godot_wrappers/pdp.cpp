#include "pdp.hpp"
#include <godot_cpp/core/class_db.hpp>

void PDP::_bind_methods() {
    ClassDB::bind_method(D_METHOD("to_string"), &PDP::to_string);

    ClassDB::bind_method(D_METHOD("get_price"), &PDP::get_price);
    ClassDB::bind_method(D_METHOD("get_supply"), &PDP::get_supply);
    ClassDB::bind_method(D_METHOD("get_demand"), &PDP::get_demand);
    ClassDB::bind_method(D_METHOD("get_buy_orders"), &PDP::get_buy_orders);
    ClassDB::bind_method(D_METHOD("get_sell_orders"), &PDP::get_sell_orders);

    ClassDB::bind_method(D_METHOD("set_price", "price"), &PDP::set_price);
    ClassDB::bind_method(D_METHOD("set_supply", "supply"), &PDP::set_supply);
    ClassDB::bind_method(D_METHOD("set_demand", "demand"), &PDP::set_demand);
    ClassDB::bind_method(D_METHOD("set_buy_orders", "buy_orders"), &PDP::set_buy_orders);
    ClassDB::bind_method(D_METHOD("set_sell_orders", "sell_orders"), &PDP::set_sell_orders);

    ADD_PROPERTY(PropertyInfo(Variant::INT, "price"), "set_price", "get_price");
    ADD_PROPERTY(PropertyInfo(Variant::INT, "supply"), "set_supply", "get_supply");
    ADD_PROPERTY(PropertyInfo(Variant::INT, "demand"), "set_demand", "get_demand");

    ADD_PROPERTY(PropertyInfo(Variant::ARRAY, "buy_orders"), "set_buy_orders", "get_buy_orders");
    ADD_PROPERTY(PropertyInfo(Variant::ARRAY, "sell_orders"), "set_sell_orders", "get_sell_orders");

}

float PDP::get_price() const {
    return price;
}

int32_t PDP::get_supply() const {
    return supply;
}

int32_t PDP::get_demand() const {
    return demand;
}

Array PDP::get_buy_orders() const {
    return buy_orders;
}

Array PDP::get_sell_orders() const {
    return sell_orders;
}

void PDP::set_price(float p_price) {
    price = p_price;
}

void PDP::set_supply(int32_t p_supply) {
    supply = p_supply;
}

void PDP::set_demand(int32_t p_demand) {
    demand = p_demand;
}

void PDP::set_buy_orders(Array p_buy_orders) {
    buy_orders = p_buy_orders;
}

void PDP::set_sell_orders(Array p_sell_orders) {
    sell_orders = p_sell_orders;
}