#include "../singletons/terminal_map.hpp"
#include "ai_factory.hpp"

void AiFactory::_bind_methods() {}

AiFactory::AiFactory(): Factory() {}

AiFactory::~AiFactory() {

}

AiFactory::AiFactory(Vector2i new_location, int player_owner, Dictionary new_inputs, Dictionary new_outputs): Factory(new_location, player_owner, new_inputs, new_outputs) {}

Ref<AiFactory> AiFactory::create(Vector2i new_location, int player_owner, Dictionary new_inputs, Dictionary new_outputs) {
    return Ref<AiFactory>(memnew(AiFactory(new_location, player_owner, new_inputs, new_outputs)));
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
    int amount = (buy ? inputs[type]: outputs[type]) * get_level();
    float price = get_local_price(type);
    float limit_price = buy ? price * 1.2: price * 0.8;

    if (get_cargo_amount(type) > get_max_storage() * MAX_AMOUNT_WANTED && outputs.count(type)) {
        amount *= 2;
    }
    
    if (get_order(type) == nullptr) {
        place_order(type, amount, buy, limit_price);
    } else {
        edit_order(type, amount, buy, limit_price);
    }
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
        int max_value = TerminalMap::get_instance() -> get_cargo_value_of_tile(get_location(), type);
        if (max_value < get_level_without_employment()) {
            return;
        }
		total_diff += local_pricer -> get_current_difference_from_base_price(type) - 1;
		amount += 1;
    }
	total_diff /= amount;
	//TODO: Consider changing constant
    
	if (total_diff > -0.05 && get_cost_for_upgrade() * CASH_NEEDED_MULTIPLIER < get_cash()) {
        upgrade();
    }
}

void AiFactory::consider_upgrade_secondary() {
    //TODO:
    return;
}


// Process Hooks
void AiFactory::AiFactory::day_tick() {
    Factory::day_tick();
}

void AiFactory::month_tick() {
    Factory::month_tick();
    change_orders();
    consider_upgrade();
}