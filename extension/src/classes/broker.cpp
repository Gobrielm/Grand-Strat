#include "broker.hpp"
#include "road_depot.hpp"
#include "../singletons/terminal_map.hpp"
#include "town_utility/town_cargo.hpp"
#include "town.hpp"
#include "broker_utility/trade_interaction.hpp"
#include "broker_utility/trade_interaction_compare.hpp"
#include <algorithm>
#include <cmath>


void Broker::_bind_methods() {
    
    ClassDB::bind_method(D_METHOD("initialize", "location", "owner", "max_amount"), &Broker::initialize);
    ClassDB::bind_method(D_METHOD("can_afford", "price"), &Broker::can_afford);
    ClassDB::bind_method(D_METHOD("get_local_prices"), &Broker::get_local_prices);
    ClassDB::bind_method(D_METHOD("get_local_price", "type"), &Broker::get_local_price);
    ClassDB::bind_method(D_METHOD("get_desired_cargo", "type", "pricePer"), &Broker::get_desired_cargo);
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

    ClassDB::bind_method(D_METHOD("get_last_month_supply"), &Broker::get_last_month_supply);
    ClassDB::bind_method(D_METHOD("get_last_month_demand"), &Broker::get_last_month_demand);
}

Broker::Broker(): FixedHold() {}

Broker::Broker(const Vector2i new_location, const int player_owner, const int p_max_amount): FixedHold(new_location, player_owner, p_max_amount) {}

Broker::~Broker() {
    std::scoped_lock lock(m);
    for (auto& [key, ptr]: trade_orders) {
        memdelete(ptr);
    }
    trade_orders.clear();
    if (local_pricer) delete local_pricer;
    for (const auto& tile: connected_brokers) {
        Ref<Broker> broker = TerminalMap::get_instance() -> get_broker(tile);
        if (broker.is_null()) continue;
        broker -> remove_connected_broker(this);
    }
}

void Broker::initialize(const Vector2i new_location, const int player_owner, const int p_max_amount) {
    FixedHold::initialize(new_location, player_owner, p_max_amount);
}

LocalPriceController* Broker::get_local_pricer() const {
    return local_pricer;
}

bool Broker::can_afford(float price) const {
    return get_cash() >= price;
}

bool Broker::can_afford_unsafe(float price) const {
    return get_cash_unsafe() >= price;
}

Dictionary Broker::get_local_prices() const {
    std::scoped_lock lock(m);
    return local_pricer -> get_local_prices_dict();
}

float Broker::get_local_price(int type) const {
    std::scoped_lock lock(m);
    return local_pricer->get_local_price(type);
}

float Broker::get_local_price_unsafe(int type) const {
    return local_pricer->get_local_price(type);
}

int Broker::get_desired_cargo(int type, float pricePer) const {
    std::scoped_lock lock(m);
    return get_desired_cargo_unsafe(type, pricePer);
}

int Broker::get_desired_cargo_unsafe(int type, float pricePer) const {
    if (trade_orders.count(type)) {
        TradeOrder* order = trade_orders.at(type);
        if (order->is_buy_order() && order->get_limit_price() >= pricePer) {
            int canGet = std::min(int(max_amount - storage.at(type)), int(get_cash_unsafe() / pricePer));
            return std::min(order->get_amount(), canGet);
        }
    }
    return 0;
}

//For buying
bool Broker::is_price_acceptable(int type, float pricePer) const {
    return get_local_price(type) * 1.1 >= pricePer; // Within 10% of local price
}

void Broker::buy_cargo(int type, int amount, float price, int p_terminal_id) {
    Ref<Firm> firm = TerminalMap::get_instance()->get_terminal_as<Firm>(p_terminal_id);
    int total = std::round(amount * price);
    add_cargo_ignore_accepts(type, amount);
    remove_cash(total);

    if (firm.is_valid()) {
        firm->add_cash(total);
        Ref<Broker> broker = Ref<Broker>(firm);
        if (broker.is_valid()) {
            broker->report_change_in_cash(total);
        }
    } else {
        print_error("Broker recieved cargo from null broker");
    }

    report_change_in_cash(-total);
}

void Broker::buy_cargo(const TownCargo* cargo) {
    //TODO: DEAL WITH FEES
    buy_cargo(cargo->type, cargo->amount, cargo->price, cargo->terminal_id);
}

int Broker::sell_cargo(int type, int amount) {
    return transfer_cargo(type, amount);
}

void Broker::report_change_in_cash(float amount) {
    std::scoped_lock lock(m);
    change_in_cash += amount;
}

float Broker::add_cargo(int type, float amount) { // If amount is ever negitive, it will break
    return FixedHold::add_cargo(type, amount);
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
    Vector2i loc = broker->get_location();
    std::scoped_lock lock(m);
    connected_brokers.erase(loc);
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

void Broker::distribute_type(int type) {
    if (int(get_cargo_amount(type)) == 0) return;
    std::set<TradeInteraction*, TradeInteractionPtrCompare> brokers_to_sell_to = get_brokers_to_distribute_to(type); // Sorted by highest price

    int current_amount = get_cargo_amount(type);
    for (auto it = brokers_to_sell_to.begin(); it != brokers_to_sell_to.end(); it++) {
        TradeInteraction* top = *it;
        if (current_amount > 0) {
            distribute_type_to_broker(type, top->main_buyer, top->middleman);
        }
        delete top;
        current_amount = get_cargo_amount(type);
    }
    
}

std::set<TradeInteraction*, TradeInteractionPtrCompare> Broker::get_brokers_to_distribute_to(int type) {
    std::set<TradeInteraction*, TradeInteractionPtrCompare> brokers_to_sell_to; // Sorted by highest price
    std::unordered_set<int> s; // Broker locations already looked at
    s.insert(terminal_id);

    for (const auto& tile : connected_brokers) {
        Ref<Broker> broker = TerminalMap::get_instance() -> get_broker(tile);
        if (broker.is_null()) continue;
        add_broker_to_sorted_set(type, s, brokers_to_sell_to, new TradeInteraction(get_price_average(type, broker), broker));
    }
    for (const auto& tile: connected_stations) {
        Ref<RoadDepot> road_depot = TerminalMap::get_instance() -> get_terminal_as<RoadDepot>(tile);
        if (road_depot.is_null()) continue;
        for (auto& broker: road_depot->get_available_brokers(type)) {
            add_broker_to_sorted_set(type, s, brokers_to_sell_to, new TradeInteraction(get_price_average(type, broker), broker));
        }
    }
    return brokers_to_sell_to;
}

float Broker::get_price_average(int type, Ref<Broker> other) const {
    return (get_local_price(type) + other->get_local_price(type)) / 2.0;
}

void Broker::add_broker_to_sorted_set(int type, std::unordered_set<int> &s, std::set<TradeInteraction*, TradeInteractionPtrCompare> &trade_interactions, TradeInteraction* trade_interaction) {
    Ref<Broker> broker = trade_interaction->main_buyer;
    if (!s.count(broker->get_terminal_id()) && broker->get_desired_cargo(type, trade_interaction->price) > 0) {
        trade_interactions.insert(trade_interaction);
        s.insert(broker->get_terminal_id());
    } else {
        delete trade_interaction;
    }
}

void Broker::distribute_type_to_broker(int type, Ref<Broker> otherBroker, Ref<RoadDepot> road_depot) { 
    float price1 = get_local_price(type);
    float price2 = otherBroker->get_local_price(type);
    float price = (price1 + price2) / 2;

    if (!is_price_acceptable(type, price) || !otherBroker->is_price_acceptable(type, price)) return;
    int desired = otherBroker->get_desired_cargo(type, price);

    int amount = std::min(desired, int(get_cargo_amount(type))); 
    amount = sell_cargo(type, amount);
    if (amount > 0) {
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

void Broker::survey_broad_market(int type) { // Doesn't include towns since they add demand seperately
    std::unordered_set<Vector2i, godot_helpers::Vector2iHasher> s;
    auto terminal_map = TerminalMap::get_instance();
    s.insert(get_location());
    
    for (const auto& tile : connected_brokers) {
        Ref<Broker> broker = TerminalMap::get_instance() -> get_broker(tile);
        if (broker.is_null() && !(terminal_map -> is_town(tile))) continue;
        s.insert(tile);
        survey_broker_market(type, broker);
    }

    for (const auto& tile: connected_stations) {
        Ref<RoadDepot> road_depot = TerminalMap::get_instance() -> get_terminal_as<RoadDepot>(tile);
        if (road_depot.is_null()) continue;
        
        std::vector<Ref<Broker>> other_brokers = road_depot->get_available_brokers(type);
        for (const auto &broker: other_brokers) {
            Vector2i cell = broker->get_location();
            if (!s.count(cell) && !(terminal_map -> is_town(cell))) {
                s.insert(cell);
                survey_broker_market(type, broker);
            }
        }
    }
}

void Broker::survey_broker_market(int type, Ref<Broker> broker) {
    if (!broker->does_accept(type)) return;
        
    if (Ref<Town>(broker).is_null()) {
        float local_price = get_local_price(type);
        float o_local_price = broker->get_local_price(type);
        add_surveyed_demand(type, o_local_price, broker->get_desired_cargo(type, o_local_price));
        broker->add_surveyed_supply(type, local_price, get_desired_cargo(type, local_price));
    } else {
        auto town = Ref<Town>(broker);
        auto demand_map = town->get_demand_at_different_prices(type);
        for (const auto& [ten_price, amount]: demand_map) {
            add_surveyed_demand(type, ten_price / 10.0f, amount);
        }
    }

}

float Broker::get_diff_between_demand_and_supply(int type) const {
    std::scoped_lock lock(m);
    return local_pricer->get_demand(type) - local_pricer->get_supply(type);
}

void Broker::add_surveyed_demand(int type, float price, float amount) {
    std::scoped_lock lock(m);
    local_pricer->add_demand(type, price, amount);
}

void Broker::add_surveyed_supply(int type, float price, float amount) {
    std::scoped_lock lock(m);
    local_pricer->add_supply(type, price, amount);
}

void Broker::add_surveyed_demand_unsafe(int type, float price, float amount) {
    local_pricer->add_demand(type, price, amount);
}

void Broker::add_surveyed_supply_unsafe(int type, float price, float amount) {
    local_pricer->add_supply(type, price, amount);
}

float Broker::get_diff_between_demand_and_supply_unsafe(int type) const {
    return local_pricer->get_demand(type) - local_pricer->get_supply(type);
}

float Broker::get_demand_at_price_unsafe(int type, float price) const {
    return local_pricer->get_demand_at_price(type, price);
}

Dictionary Broker::get_last_month_supply() const {
    std::scoped_lock lock(m);
    return local_pricer->get_last_month_supply_dict();
}

Dictionary Broker::get_last_month_demand() const {
    std::scoped_lock lock(m);
    return local_pricer->get_last_month_demand_dict();
}