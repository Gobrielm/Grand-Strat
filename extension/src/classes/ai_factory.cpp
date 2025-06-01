
#include "ai_factory.hpp"

void AiFactory::_bind_methods() {

}

AiFactory::AiFactory(): Factory() {}

AiFactory::~AiFactory() {

}

AiFactory::AiFactory(Vector2i new_location, int player_owner, Dictionary new_inputs, Dictionary new_outputs): Factory(new_location, player_owner, new_inputs, new_outputs) {}

Terminal* AiFactory::create(Vector2i new_location, int player_owner, Dictionary new_inputs, Dictionary new_outputs) {
    return memnew(AiFactory(new_location, player_owner, new_inputs, new_outputs));
}

void AiFactory::initialize(Vector2i new_location, int player_owner, Dictionary new_inputs, Dictionary new_outputs) {
    Factory::initialize(new_location, player_owner, new_inputs, new_outputs);
}

//Orders
void AiFactory::change_orders() {
    change_buy_orders();
    change_sell_orders();
}

void AiFactory::change_buy_orders() {
    for (auto& [type, __]: inputs) {
        change_order(type, true);
    }
}

void AiFactory::change_sell_orders() {
    for (auto& [type, __]: outputs) {
        change_order(type, false);
    }
}

void AiFactory::change_order(int type, bool buy) {
    int amount = buy ? inputs[type]: outputs[type];
    float price = get_local_price(type);
    float max_price = buy ? price * 1.2: price * 0.8;

    if (get_order(type) == nullptr) {
        place_order(type, amount, buy, max_price);
    }

    TradeOrder* order = get_order(type);

    if (get_cargo_amount(type) > get_max_storage() * MAX_AMOUNT_WANTED && outputs.count(type)) {
        amount = amount * 2;
    }

    order->change_amount(amount);
}

//Upgrades
void AiFactory::consider_upgrade() {
    if (inputs.size() == 0) {
        consider_upgrade_primary();
    } else {
        consider_upgrade_secondary();
    }
}

void AiFactory::consider_upgrade_primary() {
    float total_diff = 0.0;
	int amount = 0;
	for (auto& [type, __]: outputs) {
		total_diff += local_pricer -> get_percent_difference(type);
		amount += 1;
    }
	total_diff /= amount;
	//TODO: Consider changing constant
	if (total_diff > -0.05 && get_cost_for_upgrade() < get_cash() * CASH_NEEDED_MULTIPLIER && rand() % 5 == 0) {
        upgrade();
    }
}

void AiFactory::consider_upgrade_secondary() {
    //TODO:
    return;
}


// Process Hooks
void AiFactory::AiFactory::day_tick() {
    change_orders();
    Factory::day_tick();
}

void AiFactory::month_tick() {
    Factory::month_tick();
    consider_upgrade();
}