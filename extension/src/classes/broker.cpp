#include "broker.hpp"
#include <algorithm>
#include <cmath>
#include <cassert>


void Broker::_bind_methods() {
    ClassDB::bind_static_method(get_class_static(), D_METHOD("create", "new_location", "player_owner", "max_amount"), &Broker::create);
    
    ClassDB::bind_method(D_METHOD("initialize", "location", "owner", "max_amount"), &Broker::initialize);
    ClassDB::bind_method(D_METHOD("can_afford", "price"), &Broker::can_afford);
    ClassDB::bind_method(D_METHOD("get_local_prices"), &Broker::get_local_prices);
    ClassDB::bind_method(D_METHOD("get_local_price", "type"), &Broker::get_local_price);
    ClassDB::bind_method(D_METHOD("get_desired_cargo", "type", "pricePer"), &Broker::get_desired_cargo);
    ClassDB::bind_method(D_METHOD("get_desired_cargo_from_train", "type"), &Broker::get_desired_cargo_from_train);
    ClassDB::bind_method(D_METHOD("is_price_acceptable", "type", "pricePer"), &Broker::is_price_acceptable);
    ClassDB::bind_method(D_METHOD("buy_cargo", "type", "amount", "price"), &Broker::buy_cargo);
    ClassDB::bind_method(D_METHOD("sell_cargo", "type", "amount", "price"), &Broker::sell_cargo);
    ClassDB::bind_method(D_METHOD("place_order", "type", "amount", "buy", "max_price"), &Broker::place_order);
    ClassDB::bind_method(D_METHOD("edit_order", "type", "amount", "buy", "max_price"), &Broker::edit_order);
    ClassDB::bind_method(D_METHOD("get_order", "type"), &Broker::get_order);
    ClassDB::bind_method(D_METHOD("remove_order", "type"), &Broker::remove_order);
    ClassDB::bind_method(D_METHOD("add_connected_broker", "broker"), &Broker::add_connected_broker);
    ClassDB::bind_method(D_METHOD("remove_connected_broker", "broker"), &Broker::remove_connected_broker);
    ClassDB::bind_method(D_METHOD("report_attempt", "type", "amount"), &Broker::report_attempt);
    ClassDB::bind_method(D_METHOD("get_orders_dict"), &Broker::get_orders_dict);
    ClassDB::bind_method(D_METHOD("get_connected_brokers"), &Broker::get_connected_brokers);

}

Terminal* Broker::create(const Vector2i new_location, const int player_owner, const int p_max_amount) {
    return memnew(Broker(new_location, player_owner, p_max_amount));
}

Broker::Broker(): FixedHold() {}

Broker::Broker(const Vector2i new_location, const int player_owner, const int p_max_amount): FixedHold(new_location, player_owner, p_max_amount) {}

Broker::~Broker() {
    for (auto& [key, ptr]: trade_orders) {
        delete ptr;
    }
    trade_orders.clear();
}

void Broker::initialize(const Vector2i new_location, const int player_owner, const int p_max_amount) {
    FixedHold::initialize(new_location, player_owner, p_max_amount);
}

bool Broker::can_afford(int price) const {
    return get_cash() >= price;
}

Dictionary Broker::get_local_prices() const {
    return local_pricer -> get_local_prices();
}

float Broker::get_local_price(int type) const {
    return local_pricer->get_local_price(type);
}

int Broker::get_desired_cargo(int type, float pricePer) const {
    if (trade_orders.count(type)) {
        if (trade_orders.at(type)->is_buy_order() && is_price_acceptable(type, pricePer)) {
            int canGet = std::min(get_max_storage() - get_cargo_amount(type), get_amount_can_buy(pricePer));
            return std::min(trade_orders.at(type)->get_amount(), canGet);
        }
    }
    return 0;
}

int Broker::get_desired_cargo_from_train(int type) const {
    if (does_accept(type)) {
        return std::min(get_max_storage() - get_cargo_amount(type), get_amount_can_buy(get_local_price(type)));
    }
    return 0;
}

//For buying
bool Broker::is_price_acceptable(int type, float pricePer) const {
    return trade_orders.at(type)->get_limit_price() >= pricePer;
}

void Broker::buy_cargo(int type, int amount, float price) {
    add_cargo_ignore_accepts(type, amount);
    int total = std::round(amount * price);
    remove_cash(total);
    change_in_cash -= total;
    local_pricer->report_change(type, amount);
}

int Broker::sell_cargo(int type, int amount, float price) {
    int transferred = transfer_cargo(type, amount);
    int total = std::round(price * transferred);
    add_cash(total);
    change_in_cash += total;
    local_pricer->report_change(type, -transferred);
    return transferred;
}

void Broker::place_order(int type, int amount, bool buy, float maxPrice) {
    trade_orders[type] = memnew(TradeOrder(type, amount, buy, maxPrice));
}

void Broker::edit_order(int type, int amount, bool buy, float maxPrice) {
    if (trade_orders.count(type)) {
        TradeOrder* order = trade_orders[type];
        order->change_buy(buy);
        order->change_amount(amount);
        order->set_max_price(maxPrice);
    } else {
        place_order(type, amount, buy, maxPrice);
    }
}

TradeOrder* Broker::get_order(int type) const {
    if (trade_orders.count(type) == 1) return trade_orders.at(type);
    return nullptr;
}

std::unordered_map<int, TradeOrder*> Broker::get_orders() {
    return trade_orders;
}

Dictionary Broker::get_orders_dict() {
    Dictionary d;
    for (const auto &[type, order]: trade_orders) {
        d[type] = order;
    }
    return d;
}

void Broker::remove_order(int type) {
    if (trade_orders.count(type)) {
        memdelete(trade_orders[type]);
        trade_orders.erase(type);
    }
}

void Broker::add_connected_broker(Broker* broker) {
    connected_brokers[broker->get_location()] = broker;
}

void Broker::remove_connected_broker(const Broker* broker) {
    connected_brokers.erase(broker->get_location());
}

Dictionary Broker::get_connected_brokers() {
    Dictionary d;
    for (const auto &[tile, broker]: connected_brokers) {
        d[tile] = broker;
    }
    return d;
}

void Broker::distribute_cargo() {
    assert(false && "Default Implementation");
}

void Broker::distribute_from_order(const TradeOrder* order) {
    for (const auto& [__, broker] : connected_brokers) {
        if (broker->does_accept(order->get_type())) {
            distribute_to_order(broker, order);
        }
    }
}

void Broker::distribute_to_order(Broker* otherBroker, const TradeOrder* order) {
    int type = order->get_type();
    float price1 = get_local_price(type);
    float price2 = otherBroker->get_local_price(type);
    float price = std::max(price1, price2) - std::abs(price1 - price2) / 2.0f;

    if (!order->is_price_acceptable(price) || !otherBroker->is_price_acceptable(type, price)) return;

    int desired = otherBroker->get_desired_cargo(type, price);
    otherBroker->report_attempt(type, desired);
    report_attempt(type, -order->get_amount());

    int amount = std::min(desired, order->get_amount());

    if (amount > 0) {
        amount = sell_cargo(type, amount, price);
        otherBroker->buy_cargo(type, amount, price);
    }
}

void Broker::report_attempt(int type, int amount) {
    if (local_pricer) {
        local_pricer->report_attempt(type, amount);
    }
}
