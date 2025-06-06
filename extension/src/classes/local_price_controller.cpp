#include "local_price_controller.hpp"

#include <godot_cpp/core/class_db.hpp>
#include "../singletons/cargo_info.hpp"

using namespace godot;

std::vector<float> LocalPriceController::base_prices = {};

void LocalPriceController::_bind_methods() {}

LocalPriceController::LocalPriceController() {
    for (const auto &price: base_prices) {
        local_prices.push_back(price);
        supply.push_back(0);
        demand.push_back(0);
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

void LocalPriceController::add_demand(int type, int amount) { demand[type] += amount; }
void LocalPriceController::add_supply(int type, int amount) { supply[type] += amount; }
int LocalPriceController::get_demand(int type) const { return demand.at(type); }
int LocalPriceController::get_supply(int type) const { return supply.at(type); }

const std::vector<int>& LocalPriceController::get_demand() const { return demand; }
const std::vector<int>& LocalPriceController::get_supply() const { return supply; }
const std::vector<int>& LocalPriceController::get_last_month_demand() const { return last_month_demand; }
const std::vector<int>& LocalPriceController::get_last_month_supply() const { return last_month_supply; }

float LocalPriceController::get_local_price(int type) const {
    return local_prices.at(type);
}

float LocalPriceController::get_base_price(int type) const {
    return base_prices.at(type);
}

void LocalPriceController::adjust_prices() {
    int type = 0;
	for (const auto& base_price: base_prices) {
        adjust_cargo_price(type, base_price);
        type++;
    }
}

void LocalPriceController::adjust_cargo_price(int type, float base_price) {
    

    float diff_from_base = get_current_difference_from_base_price(type);

    diff_from_base = std::min(diff_from_base, 1.5f);
    diff_from_base = std::max(diff_from_base, 0.5f);

    float current_diff_from_base = get_difference_from_base_price(type, last_month_supply, last_month_demand);

    float change_of_price = (diff_from_base - current_diff_from_base) * MARKET_CHANGE_RATE; // Discrepancy_of_price

    change_of_price = change_of_price > 0 ? std::max(change_of_price, 0.01f) : std::min(change_of_price, -0.01f);

    local_prices[type] = base_price * (current_diff_from_base + change_of_price);


    last_month_supply[type] = supply[type];
    supply[type] = 0;
    last_month_demand[type] = demand[type];
    demand[type] = 0;
}


float LocalPriceController::get_difference_from_base_price(int type, std::vector<int> &p_supply, std::vector<int> &p_demand) const {
    //If no demand, then make price very cheap
    return 1 + p_demand[type] == 0 ? -1: float(p_demand[type] - p_supply[type]) / p_demand[type]; //+>1 if demand > supply
}

float LocalPriceController::get_current_difference_from_base_price(int type) {
    return get_difference_from_base_price(type, supply, demand);
}

Dictionary LocalPriceController::get_local_prices() {
    Dictionary d;
    for (int type = 0; type < local_prices.size(); type++) {
        d[type] = local_prices[type];
    }
    return d;
}