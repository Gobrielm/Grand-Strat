
#include "town_market.hpp"
#include "local_price_controller.hpp"

TownMarket::TownMarket(): Hold() {
    const auto m = LocalPriceController::get_base_prices();
    for (const auto& [type, price]: m) {
        prices[type] = price;
        supply.push_back(0);
        demand.push_back(0);
    }
}

std::vector<int>& TownMarket::get_supply() {
    return supply;
}

void TownMarket::add_cash(float amount) {
    cash += amount;
}

void TownMarket::remove_cash(float amount) {
    cash -= amount;
}

float TownMarket::get_cash() {
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
    return prices[type];
}

bool TownMarket::is_price_acceptable(int type, float price) {
    return get_local_price(type) >= price;
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
    const auto m = LocalPriceController::get_base_prices();
	for (const auto& [type, base_price]: m) {

        float percent_diff = float(demand[type] - supply[type]) / demand[type]; //+>1 if demand > supply

        float diff_from_base = 1 + percent_diff;

		diff_from_base = std::min(diff_from_base, 1.5f);
		diff_from_base = std::max(diff_from_base, 0.5f);
		prices[type] = base_price * diff_from_base;
		supply[type] = 0;
		demand[type] = 0;
    }
}

void TownMarket::month_tick() {
    adjust_prices();
}