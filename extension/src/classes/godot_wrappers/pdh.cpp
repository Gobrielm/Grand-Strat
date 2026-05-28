#include "pdh.h"
#include <godot_cpp/core/class_db.hpp>

void PDH::_bind_methods() {
    ClassDB::bind_method(D_METHOD("to_string"), &PDH::to_string);

    ClassDB::bind_method(D_METHOD("get_price"), &PDH::get_price);
    ClassDB::bind_method(D_METHOD("get_supply"), &PDH::get_supply);
    ClassDB::bind_method(D_METHOD("get_demand"), &PDH::get_demand);
    ClassDB::bind_method(D_METHOD("get_sale_history"), &PDH::get_sale_history);

    ClassDB::bind_method(D_METHOD("set_price", "price"), &PDH::set_price);
    ClassDB::bind_method(D_METHOD("set_supply", "supply"), &PDH::set_supply);
    ClassDB::bind_method(D_METHOD("set_demand", "demand"), &PDH::set_demand);
    ClassDB::bind_method(D_METHOD("set_sale_history", "sale_history"), &PDH::set_sale_history);

    ADD_PROPERTY(PropertyInfo(Variant::INT, "price"), "set_price", "get_price");
    ADD_PROPERTY(PropertyInfo(Variant::INT, "supply"), "set_supply", "get_supply");
    ADD_PROPERTY(PropertyInfo(Variant::INT, "demand"), "set_demand", "get_demand");

    ADD_PROPERTY(PropertyInfo(Variant::ARRAY, "sale_history"), "set_sale_history", "get_sale_history");

}

float PDH::get_price() const {
    return price;
}

int32_t PDH::get_supply() const {
    return supply;
}

int32_t PDH::get_demand() const {
    return demand;
}

Array PDH::get_sale_history() const {
    return sale_history;
}

void PDH::set_price(float p_price) {
    price = p_price;
}

void PDH::set_supply(int32_t p_supply) {
    supply = p_supply;
}

void PDH::set_demand(int32_t p_demand) {
    demand = p_demand;
}

void PDH::set_sale_history(Array p_sale_history) {
    sale_history = p_sale_history;
}