#include "../singletons/terminal_map.hpp"
#include "ai_factory.hpp"
#include "factory_utility/recipe.hpp"

void AiFactory::_bind_methods() {
    ClassDB::bind_method(D_METHOD("day_tick"), &AiFactory::day_tick);
    ClassDB::bind_method(D_METHOD("month_tick"), &AiFactory::month_tick);
}

AiFactory::AiFactory(): Factory() {}

AiFactory::~AiFactory() {

}

AiFactory::AiFactory(Vector2i new_location, int player_owner, Recipe* p_recipe): Factory(new_location, player_owner, p_recipe) {}

//Orders
void AiFactory::change_orders() {
    change_buy_orders();
    change_sell_orders();
}

void AiFactory::change_buy_orders() {
    for (auto& [type, __]: get_inputs()) {
        change_order(type, true);
    }
}

void AiFactory::change_sell_orders() {
    for (auto& [type, __]: get_outputs()) {
        change_order(type, false);
    }
}

void AiFactory::change_order(int type, bool buy) {
    std::unordered_map<int, float> outputs = get_outputs();
    int amount = round((buy ? get_inputs()[type]: outputs[type]) * get_level() * 1.2);
    float price = get_local_price(type);
    float limit_price = buy ? price * 1.2: price * 0.8;
    
    if (get_order(type) == nullptr) {
        place_order(type, amount, buy, limit_price);
    } else {
        edit_order(type, amount, buy, limit_price);
    }
}

//Upgrades
void AiFactory::consider_upgrade() {
    if (get_inputs().size() == 0) {
        consider_upgrade_primary();
    } else {
        consider_upgrade_secondary();
    }
}

void AiFactory::consider_upgrade_primary() {
    float gross_prof = get_theoretical_gross_profit() - (get_wage() * recipe->get_pops_needed_num());
    
	if (gross_prof > 0 && get_cost_for_upgrade() * CASH_NEEDED_MULTIPLIER < get_cash()) {
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
    // change_orders();
    consider_upgrade();
}