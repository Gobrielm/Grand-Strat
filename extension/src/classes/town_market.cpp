
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
    ClassDB::bind_method(D_METHOD("is_price_acceptable", "type", "price"), &TownMarket::is_price_acceptable);
    ClassDB::bind_method(D_METHOD("get_desired_cargo", "type", "price"), &TownMarket::get_desired_cargo);
    ClassDB::bind_method(D_METHOD("month_tick"), &TownMarket::month_tick);
}

TownMarket::TownMarket(): Broker() {
    set_max_storage(DEFAULT_MAX_STORAGE);
    for (int type = 0; type < LocalPriceController::get_base_prices().size(); type++) {
        add_accept(type);
    }
    create_storage();
}

void TownMarket::create_storage() {
    local_pricer = memnew(LocalPriceController);
}

const std::vector<int>& TownMarket::get_supply() {
    return local_pricer -> get_supply();
}

const std::vector<int>& TownMarket::get_demand() {
   return local_pricer -> get_demand();
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
    int supply = local_pricer->get_supply(type);
    int demand = local_pricer->get_demand(type);
    if (supply == 0)
		return 5;
	return float(demand / supply);
}

bool TownMarket::is_price_acceptable(int type, float price) const {
    return local_pricer -> get_local_price(type) >= price;
}

int TownMarket::get_desired_cargo(int type, float price) const {
    if (is_price_acceptable(type, price)) {
		int amount_could_get = std::min(get_max_storage() - get_cargo_amount(type), get_amount_can_buy(price));
		return std::min(local_pricer -> get_last_month_demand(type), amount_could_get);
    }
	return 0;
}

void TownMarket::month_tick() {
    local_pricer -> adjust_prices();
}