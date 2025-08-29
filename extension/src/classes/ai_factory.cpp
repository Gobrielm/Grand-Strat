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

//Upgrades
void AiFactory::consider_upgrade() {
    if (get_inputs().size() == 0) {
        consider_upgrade_primary();
    } else {
        consider_upgrade_secondary();
    }
}

void AiFactory::consider_upgrade_primary() {
    {
        std::scoped_lock lock(m);
        if (recipe->get_employment_rate() != 1) return;
    }
    float gross_prof = get_real_gross_profit(12);
	if (gross_prof > 0 && CASH_NEEDED < get_cash() && !is_constructing()) {
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
    // consider_upgrade();
}