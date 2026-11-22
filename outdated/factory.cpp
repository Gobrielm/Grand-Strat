#include "factory.hpp"

void Factory::_bind_methods() {
    ClassDB::bind_method(D_METHOD("day_tick"), &Factory::day_tick);
    ClassDB::bind_method(D_METHOD("month_tick"), &Factory::month_tick);
}


Factory::Factory(): FactoryTemplate() {}

Factory::~Factory() {}

Factory::Factory(Vector2i new_location, int player_owner, Recipe* p_recipe): FactoryTemplate(new_location, player_owner, p_recipe) {
}

// Process Hooks
void Factory::day_tick() {
    create_recipe();
    distribute_cargo();
}

void Factory::month_tick() {
    FactoryTemplate::month_tick();
    {
        std::scoped_lock lock(m);
        local_pricer->update_local_prices();
    }
}