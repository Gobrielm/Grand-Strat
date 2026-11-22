#include "isolated_broker.hpp"
#include "town.hpp"
#include "factory_utility/recipe.hpp"
#include "../singletons/cargo_info.hpp"
#include "../singletons/terminal_map.hpp"
#include "../singletons/pop_manager.hpp"
#include "../singletons/data_collector.hpp"

void IsolatedBroker::_bind_methods() {

}

IsolatedBroker::IsolatedBroker(): Firm(Vector2i(0, 0), 0), local_town(Vector2i(0, 0)) {
    std::scoped_lock lock(m);
    for (int i = 0; i < CargoInfo::get_instance()->get_number_of_goods(); i++) {
        storage[i] = 0;
    }
}

IsolatedBroker::IsolatedBroker(Vector2i p_location, int p_owner): Firm(p_location, p_owner), local_town(Vector2i(0, 0)) {
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
    if (town.is_null()) return 0;
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
    auto pop_manager = PopManager::get_instance();
    float wage = get_wage();
    std::unordered_map<int, PopTypes> employees;
    {
        std::scoped_lock lock(m);
        employees = recipe->get_employee_ids();
    }
    for (const auto& [pop_id, __] : employees) {
        pop_manager->pay_pop(pop_id, transfer_cash(wage));
        give_cargo_grain(pop_id);
    }
}

void IsolatedBroker::give_cargo_grain(int pop_id) {
    bool enough_grain = false;
    int grain_type = CargoInfo::get_instance()->get_cargo_type("grain");
    int amount_to_give = BasePop::get_base_need(peasant, grain_type);
    {
        std::scoped_lock lock(m);
        if (storage[grain_type] >= amount_to_give) {
            enough_grain = true;
            storage[grain_type] -= amount_to_give;
        }
    }
    if (enough_grain) PopManager::get_instance()->give_pop_cargo(pop_id, grain_type, amount_to_give);
}

void IsolatedBroker::set_local_town(Vector2i p_town) {
    local_town = p_town;
}

Ref<Town> IsolatedBroker::get_local_town() const {
    if (local_town == Vector2i(0, 0)) return Ref<Town>(nullptr);
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
        DataCollector::get_instance()->add_supply(type, amount * batch_size);
    }
    for (const auto& [type, amount]: recipe->get_inputs()) {
        storage[type] -= amount * batch_size;
    }
}

void IsolatedBroker::month_tick() {
    create_recipe();
    pay_employees();
    if (local_town != Vector2i(0, 0)) sell_cargo();
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