#include "local_price_controller.hpp"

#include <godot_cpp/core/class_db.hpp>
#include "../singletons/cargo_info.hpp"

using namespace godot;

std::vector<float> LocalPriceController::base_prices = {};

LocalPriceController::LocalPriceController() {
    int type = 0;
    for (const auto &price: base_prices) {
        current_prices[type] = price;
        type++;
    }
}

std::vector<float> LocalPriceController::get_base_prices() {
    return base_prices;
}

void LocalPriceController::set_base_prices() {
    for (int i = 0; i < CargoInfo::get_instance() -> get_base_prices().size(); i++) {
        base_prices.push_back(0);
    }
    for (const auto &[type, price]: CargoInfo::get_instance() -> get_base_prices()) {
        base_prices[type] = price;
    }
}

void LocalPriceController::update_local_prices() {
    for (const auto& [type, __]: current_prices) {
        update_local_price(type);
    }
}

void LocalPriceController::update_local_price(int type) { // Creates a weighted average of cargo sold
    if (demand[type].size() != 0) {
        current_prices[type] = get_weighted_average(demand[type]);
    } else if (supply[type].size() != 0) {
        current_prices[type] = get_weighted_average(supply[type]);
    }   

    last_month_supply[type] = supply[type];
    supply[type].clear();
    last_month_demand[type] = demand[type];
    demand[type].clear();

    ERR_FAIL_COND_MSG(std::isnan(current_prices[type]), "Type: " + CargoInfo::get_instance()->get_cargo_name(type) + " has null price!");
}

double LocalPriceController::get_weighted_average(std::unordered_map<int, float> &m) const {
    float total_weight = 0;
    double sum_of_weighted_terms = 0;
    for (const auto& [ten_price, amount]: m) {
        float weighted_ave = (ten_price / 10.0) * amount;
        total_weight += amount;
        sum_of_weighted_terms += weighted_ave;
    }   
    ERR_FAIL_COND_V_MSG(total_weight == 0, 0.0, "Total Amount sold is 0 yet was not erased!");
    return sum_of_weighted_terms / total_weight;
}

void LocalPriceController::add_demand(int type, float price, float amount) { 
    if (amount <= 0) return;
    demand[type][round(price * 10)] += amount; 
}
void LocalPriceController::add_supply(int type, float price, float amount) { // Somehow becomes undf
    if (amount <= 0) return;
    supply[type][round(price * 10)] += amount; 
}

float LocalPriceController::get_demand(int type) const {
    if (!demand.count(type)) return 0;
    float amount = 0;
    for (const auto& [__, amnt]: demand.at(type)) {
        amount += amnt;
    }
    return amount;
}

float LocalPriceController::get_supply(int type) const { 
    if (!supply.count(type)) return 0;
    float amount = 0;
    for (const auto& [__, amnt]: supply.at(type)) {
        amount += amnt;
    }
    return amount; 
}

float LocalPriceController::get_last_month_demand(int type) const { 
    if (!last_month_demand.count(type)) return 0;
    float amount = 0;
    for (const auto& [__, amnt]: last_month_demand.at(type)) {
        amount += amnt;
    }
    return amount; 
}

float LocalPriceController::get_last_month_supply(int type) const { 
    if (!last_month_supply.count(type)) return 0;
    float amount = 0;
    for (const auto& [__, amnt]: last_month_supply.at(type)) {
        amount += amnt;
    }
    return amount; 
}

std::unordered_map<int, float> LocalPriceController::get_demand() const { 
    std::unordered_map<int, float> toReturn;
    for (const auto& [type, m]: demand) {
        for (const auto& [__, amount]: m) {
            toReturn[type] += amount;
        }
    }
    return toReturn; 
}
std::unordered_map<int, float> LocalPriceController::get_supply() const { 
    std::unordered_map<int, float> toReturn;
    for (const auto& [type, m]: supply) {
        for (const auto& [__, amount]: m) {
            toReturn[type] += amount;
        }
    }
    return toReturn; 
}

std::unordered_map<int, float> LocalPriceController::get_last_month_demand() const { 
    std::unordered_map<int, float> toReturn;
    for (const auto& [type, m]: last_month_demand) {
        for (const auto& [__, amount]: m) {
            toReturn[type] += amount;
        }
    }
    return toReturn; 
}
std::unordered_map<int, float> LocalPriceController::get_last_month_supply() const { 
    std::unordered_map<int, float> toReturn;
    for (const auto& [type, m]: last_month_supply) {
        for (const auto& [__, amount]: m) {
            toReturn[type] += amount;
        }
    }
    return toReturn; 
}

Dictionary LocalPriceController::get_last_month_demand_dict() const {
    Dictionary d;
    for (const auto& [type, m]: last_month_demand) {
        float total = 0;
        for (const auto& [__, amount]: m) {
            total += amount;
        }
        d[type] = total;
    }
    return d;
}

Dictionary LocalPriceController::get_last_month_supply_dict() const {
    Dictionary d;
    for (const auto& [type, m]: last_month_supply) {
        float total = 0;
        for (const auto& [__, amount]: m) {
            total += amount;
        }
        d[type] = total;
    }
    return d;
}

float LocalPriceController::get_local_price(int type) const {
    ERR_FAIL_COND_V_MSG(!current_prices.count(type), 0, "Price is not available for type: " + CargoInfo::get_instance()->get_cargo_name(type));
    return current_prices.at(type);
}

float LocalPriceController::get_base_price(int type) {
    return base_prices.at(type);
}

std::unordered_map<int, float> LocalPriceController::get_local_prices() const {
    return current_prices;
}

Dictionary LocalPriceController::get_local_prices_dict() {
    Dictionary d;
    for (const auto& [type, price]: current_prices) {
        d[type] = price;
    }
    return d;
}