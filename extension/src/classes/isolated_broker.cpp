#include "isolated_broker.hpp"
#include "town.hpp"
#include "factory_utility/recipe.hpp"
#include "../singletons/cargo_info.hpp"
#include "../singletons/terminal_map.hpp"
#include "../singletons/province_manager.hpp"

void IsolatedBroker::_bind_methods() {

}

IsolatedBroker::IsolatedBroker(): Firm(Vector2i(0, 0), 0) {
    std::scoped_lock lock(m);
    for (int i = 0; i < CargoInfo::get_instance()->get_number_of_goods(); i++) {
        storage[i] = 0;
    }
}

IsolatedBroker::IsolatedBroker(Vector2i p_location, int p_owner): Firm(p_location, p_owner) {
    std::scoped_lock lock(m);
    for (int i = 0; i < CargoInfo::get_instance()->get_number_of_goods(); i++) {
        storage[i] = 0;
    }
}

void IsolatedBroker::add_pop(BasePop* pop) {
    {
        std::scoped_lock lock(m);
        recipe->add_pop(pop);
    }
    
    pop->employ(terminal_id, get_wage());
    pop->set_location(get_location());
    consider_upgrade();
}

std::unordered_map<int, float> IsolatedBroker::get_outputs() const {
    std::scoped_lock lock(m);
    return recipe->get_outputs();
}

std::unordered_map<int, float> IsolatedBroker::get_inputs() const {
    std::scoped_lock lock(m);
    return recipe->get_inputs();
}

float IsolatedBroker::get_level() const {
    std::scoped_lock lock(m);
    return recipe->get_level();
}

float IsolatedBroker::get_wage() const {
    float gross_profit = std::min(float(get_theoretical_gross_profit()), get_cash());

    int pops_needed = 0;
    {
        std::scoped_lock lock(m);
        pops_needed = recipe->get_pops_needed_num();
    }

    if (!pops_needed) return 0;
    
    return (gross_profit) / pops_needed;
}

float IsolatedBroker::get_theoretical_gross_profit() const {
    float available = 0;
    Ref<Town> town = get_local_town();
    int effective_level = std::max(get_level(), 1.0f);
    for (const auto &[type, amount]: get_inputs()) {
        available -= town->get_local_price(type) * amount * effective_level; // Always assume that the business will pay according to the first level
    }
    for (const auto &[type, amount]: get_outputs()) {
        available += town->get_local_price(type) * amount * effective_level;
    }
    available *= 30;
    return available;
}

void IsolatedBroker::pay_employees() {
    Ref<ProvinceManager> province_manager = ProvinceManager::get_instance();
    float wage = get_wage();
    std::unordered_map<int, PopTypes> employees;
    {
        std::scoped_lock lock(m);
        employees = recipe->get_employee_ids();
    }
    for (const auto& [pop_id, __] : employees) {
        Province* province = province_manager->get_province(province_manager->get_province_id(get_location()));
        province->pay_pop(pop_id, transfer_cash(wage));
    }
}

void IsolatedBroker::set_local_town(Vector2i p_town) {
    local_town = p_town;
}

Ref<Town> IsolatedBroker::get_local_town() const {
    return TerminalMap::get_instance()->get_town(local_town);
}

void IsolatedBroker::sell_cargo() {
    Ref<Town> town = TerminalMap::get_instance()->get_terminal_as<Town>(local_town);
    for (const auto& [type, __]: get_outputs()) {
        sell_type(town, type, storage[type]);
    }
    
}

void IsolatedBroker::sell_type(Ref<Town> town, int type, int amount) {
    float price = town->get_local_price(type);
    amount = std::min(amount, town->get_desired_cargo(type, price));
    if (amount > 0) {
        town->buy_cargo(type, amount, price, get_terminal_id());
        {
            std::scoped_lock lock(m);
            storage[type] -= amount;
        }
    }
}

double IsolatedBroker::get_batch_size() const {
    std::scoped_lock lock(m);
    double batch_size = recipe->get_level();
    for (auto& [type, amount]: recipe->get_inputs()) {
        batch_size = std::min(storage.at(type) / double(amount), batch_size);
    }
    for (auto& [type, amount]: recipe->get_outputs()) {
        batch_size = std::min((double(MAX_STORAGE) - storage.at(type)) / amount, batch_size);
    }
    return batch_size;
}

void IsolatedBroker::create_recipe() {
    double batch_size = get_batch_size();
    if (batch_size == 0) return;
    std::scoped_lock lock(m);
    for (const auto& [type, amount]: recipe->get_outputs()) {
        storage[type] += amount * batch_size;
    }
    for (const auto& [type, amount]: recipe->get_inputs()) {
        storage[type] -= amount * batch_size;
    }
}

void IsolatedBroker::month_tick() {
    create_recipe();
    sell_cargo();
    pay_employees();
}

void IsolatedBroker::consider_upgrade() {
    std::scoped_lock lock(m);
    if (recipe->get_employment_rate() > 0.8) {
        recipe->upgrade();
    }
}

void IsolatedBroker::consider_degrade() {
    std::scoped_lock lock(m);
    if (recipe->get_employment_rate() < 0.5) {
        recipe->upgrade();
    }
}