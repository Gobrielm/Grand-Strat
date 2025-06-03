#include "factory.hpp"

void Factory::_bind_methods() {
    ClassDB::bind_static_method(get_class_static(), D_METHOD("create", "new_location", "player_owner", "new_inputs", "new_outputs"), &Factory::create);
    ClassDB::bind_method(D_METHOD("initialize", "new_location", "player_owner", "new_inputs", "new_outputs"), &Factory::initialize);

}


Factory::Factory(): FactoryTemplate() {}

Factory::~Factory() {}

Factory::Factory(Vector2i new_location, int player_owner, Dictionary new_inputs, Dictionary new_outputs) {
    FactoryTemplate::initialize(new_location, player_owner, new_inputs, new_outputs);
}

Terminal* Factory::create(Vector2i new_location, int player_owner, Dictionary new_inputs, Dictionary new_outputs) {
    return memnew(Factory(new_location, player_owner, new_inputs, new_outputs));
}

void Factory::initialize(Vector2i new_location, int player_owner, Dictionary new_inputs, Dictionary new_outputs) {
    FactoryTemplate::initialize(new_location, player_owner, new_inputs, new_outputs);
}

    // Recipe
bool Factory::check_recipe() {
    return check_inputs() && check_outputs();
}

bool Factory::check_inputs() {
    for (const auto& [type, amount]: inputs) {
        if (get_cargo_amount(type) < amount) {
            return false;
        }
    }
    return true;
}

bool Factory::check_outputs() {
    for (const auto& [type, amount]: outputs) {
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
    for (auto& [type, __]: inputs) {
        local_pricer->vary_input_price(get_monthly_demand(type), type);
    }
    for (auto& [type, __]: outputs) {
        local_pricer->vary_output_price(get_monthly_supply(type), type);
    }
}