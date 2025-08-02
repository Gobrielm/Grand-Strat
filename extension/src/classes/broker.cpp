#include "broker.hpp"
#include "road_depot.hpp"
#include "../singletons/terminal_map.hpp"
#include "town_utility/town_cargo.hpp"
#include <algorithm>
#include <cmath>


void Broker::_bind_methods() {
    
    ClassDB::bind_method(D_METHOD("initialize", "location", "owner", "max_amount"), &Broker::initialize);
    ClassDB::bind_method(D_METHOD("can_afford", "price"), &Broker::can_afford);
    ClassDB::bind_method(D_METHOD("get_local_prices"), &Broker::get_local_prices);
    ClassDB::bind_method(D_METHOD("get_local_price", "type"), &Broker::get_local_price);
    ClassDB::bind_method(D_METHOD("get_desired_cargo", "type", "pricePer"), &Broker::get_desired_cargo);
    ClassDB::bind_method(D_METHOD("get_desired_cargo_from_train", "type"), &Broker::get_desired_cargo_from_train);
    ClassDB::bind_method(D_METHOD("is_price_acceptable", "type", "pricePer"), &Broker::is_price_acceptable);
    ClassDB::bind_method(D_METHOD("place_order", "type", "amount", "buy", "max_price"), &Broker::place_order);
    ClassDB::bind_method(D_METHOD("edit_order", "type", "amount", "buy", "max_price"), &Broker::edit_order);
    ClassDB::bind_method(D_METHOD("get_order", "type"), &Broker::get_order);
    ClassDB::bind_method(D_METHOD("remove_order", "type"), &Broker::remove_order);
    ClassDB::bind_method(D_METHOD("add_connected_broker", "broker"), &Broker::add_connected_broker);
    ClassDB::bind_method(D_METHOD("remove_connected_broker", "broker"), &Broker::remove_connected_broker);
    ClassDB::bind_method(D_METHOD("get_orders_dict"), &Broker::get_orders_dict);
    ClassDB::bind_method(D_METHOD("get_connected_broker_locations"), &Broker::get_connected_broker_locations);
    ClassDB::bind_method(D_METHOD("get_number_of_connected_terminals"), &Broker::get_number_of_connected_terminals);
}

Broker::Broker(): FixedHold() {}

Broker::Broker(const Vector2i new_location, const int player_owner, const int p_max_amount): FixedHold(new_location, player_owner, p_max_amount) {}

Broker::~Broker() {
    std::scoped_lock lock(m);
    for (auto& [key, ptr]: trade_orders) {
        memdelete(ptr);
    }
    trade_orders.clear();
    if (local_pricer) memdelete(local_pricer);
    for (const auto& tile: connected_brokers) {
        Ref<Broker> broker = TerminalMap::get_instance() -> get_broker(tile);
        if (broker.is_null()) continue;
        broker -> remove_connected_broker(this);
    }
}

void Broker::initialize(const Vector2i new_location, const int player_owner, const int p_max_amount) {
    FixedHold::initialize(new_location, player_owner, p_max_amount);
}

bool Broker::can_afford(int price) const {
    return get_cash() >= price;
}

Dictionary Broker::get_local_prices() const {
    std::scoped_lock lock(m);
    return local_pricer -> get_local_prices();
}

float Broker::get_local_price(int type) const {
    std::scoped_lock lock(m);
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
    std::scoped_lock lock(m);
    return trade_orders.at(type)->get_limit_price() >= pricePer;
}

void Broker::buy_cargo(int type, int amount, float price, int p_terminal_id) {
    Ref<Broker> broker = TerminalMap::get_instance()->get_terminal_as<Broker>(p_terminal_id);
    int total = std::round(amount * price);
    add_cargo_ignore_accepts(type, amount);
    remove_cash(total);
    
    broker->add_cash(total);
    broker->report_change_in_cash(total);

    report_change_in_cash(-total);
}

void Broker::buy_cargo(const TownCargo* cargo) {
    //TODO: DEAL WIHT FEES
    buy_cargo(cargo->type, cargo->amount, cargo->price, cargo->terminal_id);
}

int Broker::sell_cargo(int type, int amount) {
    return transfer_cargo(type, amount);
}

int Broker::add_cargo_ignore_accepts(int type, int amount) { //Make sure 
    {
        std::scoped_lock lock(m);
        local_pricer -> add_supply(type, amount);
    }
    return FixedHold::add_cargo_ignore_accepts(type, amount);
}

void Broker::report_change_in_cash(float amount) {
    std::scoped_lock lock(m);
    change_in_cash += amount;
}

int Broker::add_cargo(int type, int amount) { // If amount is ever negitive, it will break
    amount = FixedHold::add_cargo(type, amount);
    std::scoped_lock lock(m);
    local_pricer -> add_supply(type, amount);
    return amount;
}

void Broker::place_order(int type, int amount, bool buy, float maxPrice) {
    std::scoped_lock lock(m);
    trade_orders[type] = memnew(TradeOrder(type, amount, buy, maxPrice));
}

void Broker::edit_order(int type, int amount, bool buy, float maxPrice) {
    std::scoped_lock lock(m);
    auto it = trade_orders.find(type);
    if (it != trade_orders.end()) {
        TradeOrder* trade_order = it->second;
        trade_order->change_buy(buy);
        trade_order->change_amount(amount);
        trade_order->set_max_price(maxPrice);
    } else {
        trade_orders[type] = memnew(TradeOrder(type, amount, buy, maxPrice));
    }
}

TradeOrder* Broker::get_order(int type) const {
    std::scoped_lock lock(m);
    if (trade_orders.count(type) == 1) return trade_orders.at(type);
    return nullptr;
}

std::unordered_map<int, TradeOrder*> Broker::get_orders() {
    std::scoped_lock lock(m);
    return trade_orders;
}

Dictionary Broker::get_orders_dict() { 
    Dictionary d;
    std::scoped_lock lock(m);
    for (const auto &[type, order]: trade_orders) {
        d[type] = order;
    }
    return d;
}

void Broker::remove_order(int type) {
    std::scoped_lock lock(m);
    if (trade_orders.count(type)) {
        memdelete(trade_orders[type]);
        trade_orders.erase(type);
    }
}

void Broker::add_connected_broker(Ref<Broker> broker) {
    Vector2i tile = broker->get_location();
    std::scoped_lock lock(m);
    connected_brokers.insert(tile);
}

void Broker::remove_connected_broker(const Ref<Broker> broker) {
    std::scoped_lock lock(m);
    connected_brokers.erase(broker->get_location());
}

Dictionary Broker::get_connected_broker_locations() {
    std::scoped_lock lock(m);
    Dictionary d;
    for (const auto &tile: connected_brokers) {
        d[tile] = true;
    }
    return d;
}

void Broker::add_connected_station(const Vector2i p_location) {
    std::scoped_lock lock(m);
    connected_stations.insert(p_location);
}

void Broker::remove_connected_station(const Vector2i p_location) {
    std::scoped_lock lock(m);
    if (connected_stations.count(p_location)) {
        connected_stations.erase(p_location);
    }
}

int Broker::get_number_of_connected_terminals() const {
    std::scoped_lock lock(m);
    return connected_brokers.size() + connected_stations.size();
}

void Broker::distribute_cargo() {
    ERR_FAIL_MSG("DEFAULT IMPLEMENTATION");
}

void Broker::distribute_from_order(const TradeOrder* order) {
    std::unordered_set<Vector2i, godot_helpers::Vector2iHasher> s;

    for (const auto& tile : connected_brokers) {
        if (get_cargo_amount(order->get_type()) == 0) return;
        Ref<Broker> broker = TerminalMap::get_instance() -> get_broker(tile);
        if (broker.is_null()) continue;
        s.insert(tile);
        if (broker->does_accept(order->get_type())) {
            distribute_to_order(broker, order);
        }
    }
    for (const auto& tile: connected_stations) {
        if (get_cargo_amount(order->get_type()) == 0) return;
        Ref<RoadDepot> road_depot = TerminalMap::get_instance() -> get_terminal_as<RoadDepot>(tile);
        if (road_depot.is_null()) continue;
        distribute_to_road_depot_brokers(road_depot, order, s);
    }
    
}

void Broker::distribute_to_road_depot_brokers(Ref<RoadDepot> road_depot, const TradeOrder* order, std::unordered_set<Vector2i, godot_helpers::Vector2iHasher> &s) {
    std::vector<Ref<Broker>> other_brokers = road_depot->get_available_brokers(order->get_type());
    for (auto broker: other_brokers) {
        if (!s.count(broker->get_location())) {
            s.insert(broker->get_location());
            distribute_to_order(broker, order, road_depot);
        }
    }
}

void Broker::distribute_to_order(Ref<Broker> otherBroker, const TradeOrder* order, Ref<RoadDepot> road_depot) { 
    int type = order->get_type();
    float price1 = get_local_price(type);
    otherBroker->report_price(type, price1);
    float price2 = otherBroker->get_local_price(type);
    float price = std::max(price1, price2) - std::abs(price1 - price2) / 2.0f;

    if (!order->is_price_acceptable(price) || !otherBroker->is_price_acceptable(type, price)) return;
    int desired = otherBroker->get_desired_cargo(type, price);

    int amount = std::min(desired, order->get_amount()); 
    if (amount > 0) {
        amount = sell_cargo(type, amount);
        {
            TownCargo* cargo = new TownCargo(type, amount, price, get_terminal_id());
            if (road_depot.is_valid()) {
                cargo->add_fee_to_pay(road_depot->get_terminal_id(), road_depot->get_fee()); // FEES ADDED BUT NOT DEALT WITH
            }
            otherBroker->buy_cargo(cargo);
            delete cargo;
        }
    }
}

void Broker::report_attempt_to_sell(int type, int amount) {
    std::scoped_lock lock(m);
    if (local_pricer) {
        local_pricer->add_demand(type, amount);
    }
}

void Broker::report_demand_of_brokers(int type) {
    float price = get_local_price(type);
    std::unordered_set<Vector2i, godot_helpers::Vector2iHasher> s;
    s.insert(get_location());
    
    for (const auto& tile : connected_brokers) {
        Ref<Broker> broker = TerminalMap::get_instance() -> get_broker(tile);
        if (broker.is_null()) continue;
        s.insert(tile);
        if (broker->does_accept(type)) {
            report_attempt_to_sell(type, broker->get_desired_cargo(type, price));
        }
    }

    for (const auto& tile: connected_stations) {
        Ref<RoadDepot> road_depot = TerminalMap::get_instance() -> get_terminal_as<RoadDepot>(tile);
        if (road_depot.is_null()) continue;
        
        std::vector<Ref<Broker>> other_brokers = road_depot->get_available_brokers(type);
        for (const auto &broker: other_brokers) {
            if (!s.count(broker->get_location())) {
                s.insert(broker->get_location());
                report_attempt_to_sell(type, broker->get_desired_cargo(type, price));
            }
        }
    }
}

void Broker::report_price(int type, float price) {}