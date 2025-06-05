
#include "town_market.hpp"
#include "local_price_controller.hpp"
#include "../singletons/cargo_info.hpp"

void TownMarket::_bind_methods() {
    ClassDB::bind_method(D_METHOD("create_storage"), &TownMarket::create_storage);
    ClassDB::bind_method(D_METHOD("add_cash", "amount"), &TownMarket::add_cash);
    ClassDB::bind_method(D_METHOD("remove_cash", "amount"), &TownMarket::remove_cash);
    ClassDB::bind_method(D_METHOD("get_cash"), &TownMarket::get_cash);
    ClassDB::bind_method(D_METHOD("transfer_cash", "amount"), &TownMarket::transfer_cash);
    ClassDB::bind_method(D_METHOD("get_fulfillment", "type"), &TownMarket::get_fulfillment);
    ClassDB::bind_method(D_METHOD("report_attempt_to_sell", "type", "amount"), &TownMarket::report_attempt_to_sell);
    ClassDB::bind_method(D_METHOD("get_local_price", "type"), &TownMarket::get_local_price);
    ClassDB::bind_method(D_METHOD("is_price_acceptable", "type", "price"), &TownMarket::is_price_acceptable);
    ClassDB::bind_method(D_METHOD("get_desired_cargo", "type", "price"), &TownMarket::get_desired_cargo);
    ClassDB::bind_method(D_METHOD("buy_cargo", "type", "amount", "price"), &TownMarket::buy_cargo);
    ClassDB::bind_method(D_METHOD("sell_cargo", "type", "amount", "price"), &TownMarket::sell_cargo);
    ClassDB::bind_method(D_METHOD("month_tick"), &TownMarket::month_tick);
}

TownMarket::TownMarket(): Hold() {
    set_max_storage(DEFAULT_MAX_STORAGE);
    create_storage();
}

void TownMarket::create_storage() {
    const auto v = LocalPriceController::get_base_prices();
    for (const auto &price: v) {
        prices.push_back(price);
        supply.push_back(0);
        demand.push_back(0);
        last_month_demand.push_back(0);
        last_month_supply.push_back(0);
    }
}

std::vector<int>& TownMarket::get_supply() {
    return last_month_supply;
}

std::vector<int>& TownMarket::get_demand() {
    return last_month_demand;
}

void TownMarket::add_cash(float amount) {
    cash += amount;
}

void TownMarket::remove_cash(float amount) {
    cash -= amount;
}

float TownMarket::get_cash() const {
    return cash;
}

float TownMarket::transfer_cash(float amount) {
    amount = std::min(amount, get_cash());
    remove_cash(amount);
    return amount;
}

float TownMarket::get_fulfillment(int type) {
    if (supply[type] == 0)
		return 5;
	return float(demand[type]) / supply[type];
}

void TownMarket::report_attempt_to_sell(int type, int amount) {
    demand[type] += amount;
}

float TownMarket::get_local_price(int type) {
    if (prices.size() <= type) {
        ERR_PRINT("Prices only has size " + String::num_int64(prices.size()) + " and you accessed " + String::num_int64(type));
        return 0;
    }
    return prices[type];
}

bool TownMarket::is_price_acceptable(int type, float price) {
    return get_local_price(type) >= price;
}

int TownMarket::get_desired_cargo(int type, float price) {
    if (is_price_acceptable(type, price)) {
		int amount_could_get = std::min(get_max_storage() - get_cargo_amount(type), get_amount_can_buy(price));
		return std::min(last_month_demand[type], amount_could_get);
    }
	return 0;
}

void TownMarket::buy_cargo(int type, int amount, float price) {
    add_cargo(type, amount);
	supply[type] += amount;
	remove_cash(round(amount * price));
}

int TownMarket::sell_cargo(int type, int amount, float price) {
    amount = transfer_cargo(type, amount);
	add_cash(round(price * amount));
	return amount;
}

void TownMarket::adjust_prices() {
    const auto v = LocalPriceController::get_base_prices();
    int type = 0;
	for (const auto& base_price: v) {
        //If no demand, don't defer from base_price
        float percent_diff = demand[type] == 0 ? 0: float(demand[type] - supply[type]) / demand[type]; //+>1 if demand > supply

        float diff_from_base = 1 + percent_diff;

		diff_from_base = std::min(diff_from_base, 1.5f);
		diff_from_base = std::max(diff_from_base, 0.5f);
		prices[type] = base_price * diff_from_base;
        last_month_supply[type] = supply[type];
		supply[type] = 0;
        last_month_demand[type] = demand[type];
		demand[type] = 0;
        type++;
    }
}

void TownMarket::month_tick() {
    adjust_prices();
}