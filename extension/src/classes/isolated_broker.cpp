#include "isolated_broker.hpp"
#include "town.hpp"
#include "factory_utility/recipe.hpp"
#include "../singletons/cargo_info.hpp"
#include "../singletons/terminal_map.hpp"

void IsolatedBroker::_bind_methods() {

}

IsolatedBroker::IsolatedBroker(): Firm(Vector2i(0, 0), 0) {
    for (int i = 0; i < CargoInfo::get_instance()->get_number_of_goods(); i++) {
        std::scoped_lock lock(m);
        storage[i] = 0;
    }
}

IsolatedBroker::IsolatedBroker(Vector2i p_location, int p_owner): Firm(p_location, p_owner) {
    for (int i = 0; i < CargoInfo::get_instance()->get_number_of_goods(); i++) {
        std::scoped_lock lock(m);
        storage[i] = 0;
    }
}

void IsolatedBroker::add_pop(BasePop* pop) {
    recipe->add_pop(pop);
    consider_upgrade();
}

void IsolatedBroker::set_local_town(Vector2i p_town) {
    local_town = p_town;
}

void IsolatedBroker::sell_cargo() {
    Ref<Town> town = TerminalMap::get_instance()->get_terminal_as<Town>(local_town);
    for (const auto& [type, __]: recipe->get_outputs()) {
        sell_type(town, type, storage[type]);
    }
    
}

void IsolatedBroker::sell_type(Ref<Town> town, int type, int amount) {
    float price = town->get_local_price(type);
    amount = std::min(amount, town->get_desired_cargo(type, price));
    town->buy_cargo(type, amount, price, get_terminal_id());
    {
        std::scoped_lock lock(m);
        storage[type] -= amount;
    }
    
}

bool IsolatedBroker::check_outputs() const {
    for (const auto& [type, amount]: recipe->get_outputs()) {
        if (storage.at(type) + amount > MAX_STORAGE) {
            return false;
        }
    }
    return true;
}

bool IsolatedBroker::check_inputs() const {
    for (const auto& [type, amount]: recipe->get_inputs()) {
        if (storage.at(type) - amount < 0) {
            return false;
        }
    }
    return true;
}

void IsolatedBroker::create_recipe() {
    if (check_outputs() && check_inputs()) {
        std::scoped_lock lock(m);
        for (const auto& [type, amount]: recipe->get_outputs()) {
            storage[type] += amount;
        }
        for (const auto& [type, amount]: recipe->get_inputs()) {
            storage[type] -= amount;
        }
    }
}

void IsolatedBroker::month_tick() {
    create_recipe();
    sell_cargo();
}

void IsolatedBroker::consider_upgrade() {
    if (recipe->get_employment_rate() > 0.8) {
        recipe->upgrade();
    }
}

void IsolatedBroker::consider_degrade() {
    if (recipe->get_employment_rate() < 0.5) {
        recipe->upgrade();
    }
}