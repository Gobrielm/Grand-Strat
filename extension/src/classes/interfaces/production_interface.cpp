#include "production_interface.hpp"

ProductionInterface::ProductionInterface() {}

ProductionInterface::~ProductionInterface() {
    // for (const auto& tile: connected_brokers) {
    //     Ref<ProductionInterface> broker = TerminalMap::get_instance() -> get_broker(tile);
    //     if (broker.is_null()) continue;
    //     broker -> remove_connected_broker(this);
    // }
}

Dictionary ProductionInterface::get_local_prices() const {
    return local_pricer.get_local_prices_dict();
}

float ProductionInterface::get_local_price(int type) const {
    return local_pricer.get_local_price(type);
}

int ProductionInterface::get_desired_cargo(int type, float pricePer) const {
    // if (trade_orders.count(type)) {
    //     TradeOrder* order = trade_orders.at(type);
    //     if (order->is_buy_order() && order->get_limit_price() >= pricePer) {
    //         int canGet = std::min(int(max_amount - storage.at(type)), int(get_cash_unsafe() / pricePer));
    //         return std::min(order->get_amount(), canGet);
    //     }
    // }
    return 0;                                                                                                              
}

bool ProductionInterface::is_price_acceptable(int type, float pricePer) const {
    return get_local_price(type) * 1.1 >= pricePer || 
        get_local_price(type) * 0.9 <= pricePer; // Within 10% of local price
}

float ProductionInterface::add_cargo(int type, float amount) {
    if (amount >= 0) {
        float amt_to_take = std::min(amount, MAX_STORAGE - amount);
        storage[type] += amt_to_take;
        return amount - amt_to_take;
    }
    return amount;
}


float ProductionInterface::get_price_average(int type, const ProductionInterface& other) const {
    return (get_local_price(type) + other.get_local_price(type)) / 2.0;
}

std::unordered_map<int, float> ProductionInterface::get_last_month_demand(int type) const {
    return local_pricer.get_last_month_demand_ten_price_map(type);
}

std::unordered_map<int, float> ProductionInterface::get_last_month_supply(int type) const {
    return local_pricer.get_last_month_supply_ten_price_map(type);
}

float ProductionInterface::get_diff_between_demand_and_supply(int type) const {
    return local_pricer.get_demand(type) - local_pricer.get_supply(type);
}

void ProductionInterface::add_demand(int type, float price, float amount) {
    local_pricer.add_demand(type, price, amount);
}

void ProductionInterface::add_demand(int type, const std::unordered_map<int, float>& price_map) {
    for (const auto& [amt, price]: price_map) {
        local_pricer.add_demand(type, price, amt);
    }
}

void ProductionInterface::add_supply(int type, float price, float amount) {
    local_pricer.add_supply(type, price, amount);
}

void ProductionInterface::add_supply(int type, const std::unordered_map<int, float>& price_map) {
    for (const auto& [amt, price]: price_map) {
        local_pricer.add_supply(type, price, amt);
    }
}

float ProductionInterface::get_diff_between_demand_and_supply(int type) const {
    return local_pricer.get_demand(type) - local_pricer.get_supply(type);
}

float ProductionInterface::get_demand_at_price(int type, float price) const {
    return local_pricer.get_demand_at_price(type, price);
}

Dictionary ProductionInterface::get_last_month_supply() const {
    return local_pricer.get_last_month_supply_dict();
}

Dictionary ProductionInterface::get_last_month_demand() const {
    return local_pricer.get_last_month_demand_dict();
}