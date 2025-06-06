#include "station.hpp"

using namespace godot;

void StationWOMethods::_bind_methods() {
    ClassDB::bind_static_method(get_class_static(), D_METHOD("create", "new_location", "player_owner"), &StationWOMethods::create);
    ClassDB::bind_method(D_METHOD("initialize", "new_location", "player_owner"), &StationWOMethods::initialize);

    // Expose methods to GDScript
    ClassDB::bind_method(D_METHOD("get_local_price", "type"), &StationWOMethods::get_local_price);
    ClassDB::bind_method(D_METHOD("place_order", "type", "amount", "buy", "max_price"), &StationWOMethods::place_order);
    ClassDB::bind_method(D_METHOD("edit_order", "type", "amount", "buy", "max_price"), &StationWOMethods::edit_order);
    ClassDB::bind_method(D_METHOD("get_orders_magnitude"), &StationWOMethods::get_orders_magnitude);
    ClassDB::bind_method(D_METHOD("remove_order", "type"), &StationWOMethods::remove_order);
    ClassDB::bind_method(D_METHOD("distribute_cargo"), &StationWOMethods::distribute_cargo);
    ClassDB::bind_method(D_METHOD("day_tick"), &StationWOMethods::day_tick);

    ClassDB::bind_integer_constant(get_class_static(), "Int", "SUPPLY_DROPOFF", SUPPLY_DROPOFF);
    ClassDB::bind_integer_constant(get_class_static(), "Int", "MAX_SUPPLY_DISTANCE", MAX_SUPPLY_DISTANCE);

    GDVIRTUAL_BIND(supply_armies);

}

Terminal* StationWOMethods::create(Vector2i new_location, int player_owner) {
    return memnew(StationWOMethods(new_location, player_owner));
}

StationWOMethods::StationWOMethods() {}

StationWOMethods::StationWOMethods(Vector2i new_location, int player_owner) {
    initialize(new_location, player_owner);
}

void StationWOMethods::initialize(Vector2i new_location, int player_owner) {
    Broker::initialize(new_location, player_owner);
    local_pricer = memnew(LocalPriceController);
}

float StationWOMethods::get_local_price(int type) const {
    int amount_total = 0;
    float market_price = 0.0;

    for (const auto &[__, broker] : connected_brokers) {
        const TradeOrder* order = broker->get_order(type);
        if (order == nullptr) continue;

        amount_total += order->get_amount();
        market_price += order->get_amount() * broker->get_local_price(type);
    }
    float max_price = CargoInfo::get_instance() -> get_base_prices().at(type) * LocalPriceController::MAX_DIFF;
    return amount_total == 0 ? max_price : market_price / amount_total;
}

void StationWOMethods::place_order(int type, int amount, bool buy, float max_price) {
    add_accept(type);
    Broker::place_order(type, amount, buy, max_price);
}

void StationWOMethods::edit_order(int type, int amount, bool buy, float max_price) {
    if (get_order(type) != nullptr) {
        Broker::edit_order(type, amount, buy, max_price);
    } else {
        place_order(type, amount, buy, max_price);
    }
}

int StationWOMethods::get_orders_magnitude() {
    int total = 0;
    for (const auto &[type, order]: get_orders()) {
        total += order->get_amount();
    }
    return total;
}

void StationWOMethods::remove_order(int type) {
    if (get_order(type) != nullptr) {
        remove_accept(type);
        Broker::remove_order(type);
    }
}

void StationWOMethods::distribute_cargo() {
    supply_armies();
    
    for (const auto& [__, order] : get_orders()) {
        if (order->is_sell_order()) {
            distribute_from_order(order);
        }
    }
}

void StationWOMethods::supply_armies() {
    if (GDVIRTUAL_CALL(supply_armies)) {
        return;
    }
    ERR_FAIL_MSG("Not implemented");
}

void StationWOMethods::add_connected_broker(Broker* new_broker) {
    Broker::add_connected_broker(new_broker);
    refresh_accepts();
}

void StationWOMethods::remove_connected_broker(const Broker* new_broker) {
    Broker::remove_connected_broker(new_broker);
    refresh_accepts();
}

void StationWOMethods::refresh_accepts() {
    reset_accepts();
    for (const auto &[tile, broker] : connected_brokers) {
        add_accepts(broker);
    }
}

void StationWOMethods::add_accepts(Broker* broker) {
    int type = 0;
    for (bool status: broker->get_accepts_vector()) {
        if (status) add_accept(type);
        type++;
    }
}

void StationWOMethods::reset_accepts_train() {
    reset_accepts();
}

void StationWOMethods::day_tick() {
    distribute_cargo();
}

