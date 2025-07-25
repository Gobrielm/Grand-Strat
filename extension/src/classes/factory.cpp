#include "factory.hpp"

void Factory::_bind_methods() {

    ClassDB::bind_method(D_METHOD("day_tick"), &Factory::day_tick);
    ClassDB::bind_method(D_METHOD("month_tick"), &Factory::month_tick);

}


Factory::Factory(): FactoryTemplate() {}

Factory::~Factory() {}

Factory::Factory(Vector2i new_location, int player_owner, Recipe* p_recipe) {
    FactoryTemplate::initialize(new_location, player_owner, p_recipe);
}

    // Recipe
bool Factory::check_recipe() {
    return check_outputs() && check_inputs(); // Check outputs first, since check_inputs shouldn't be run if outputs aren't open
}

bool Factory::check_inputs() {
    bool toReturn = true;
    for (const auto& [type, amount]: get_inputs()) {
        m.lock();
        local_pricer -> add_demand(type, amount);
        m.unlock();
        if (get_cargo_amount(type) < amount) {
            toReturn = false;
        }
    }
    return toReturn;
}

bool Factory::check_outputs() {
    for (const auto& [type, amount]: get_outputs()) {
        if (get_max_storage() - get_cargo_amount(type) < amount) {
            return false;
        }
    }
    return true;
}
// Process Hooks
void Factory::day_tick() {
    if (check_recipe()) {
        create_recipe();
    }
    if (get_orders().size() != 0) {
        distribute_cargo();
    }
}

void Factory::month_tick() {
    FactoryTemplate::month_tick();
    m.lock();
    local_pricer->adjust_prices();
    m.unlock();
}