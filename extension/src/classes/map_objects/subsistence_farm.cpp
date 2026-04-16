#include "subsistence_farm.hpp"
#include <src/singletons/recipe_info.hpp>
#include <src/singletons/cargo_info.hpp>
#include "town.hpp"

// #include "../singletons/terminal_map.hpp"
// #include "../singletons/pop_manager.hpp"
// #include "../singletons/data_collector.hpp"

void SubsistenceFarm::_bind_methods() {
    ClassDB::bind_method(D_METHOD("month_tick"), &SubsistenceFarm::month_tick);
    
}

SubsistenceFarm::SubsistenceFarm(): employer() {
    RecipeInfo::convert_readable_recipe_into_recipe(
        {{}, 
        {{"grain", 11.0f}}}, 
        {{peasant, 10}}
    );
}

SubsistenceFarm::SubsistenceFarm(Vector2i p_location, int p_owner): position(std::make_pair(p_location.x, p_location.y), SUBSISTENCE_FARM), owner(p_owner) {
    recipe = RecipeInfo::convert_readable_recipe_into_recipe(
        {{}, 
        {{"grain", 11.0f}}}, 
        {{peasant, 10}}
    );
}

void SubsistenceFarm::month_tick() {
    SubsistenceFarm::month_tick();
}

void SubsistenceFarm::add_pop(BasePop* pop) {
    recipe.add_pop(pop);
    
    pop->employ(position.building_id, get_wage());
    pop->set_location(position.get_position_vector2i());
    consider_upgrade();
}

float SubsistenceFarm::get_wage(const Town& town) const {
    float gross_profit = std::min(float(get_theoretical_gross_profit(town)), capital.get_cash());

    int pops_needed = recipe.get_pops_needed_num();

    if (pops_needed == 0) return 0;
    return (gross_profit) / pops_needed;
}

float SubsistenceFarm::get_theoretical_gross_profit(const Town& town) const {
    float available = 0;

    double effective_level = std::max(recipe.get_level(), 1.0);
    for (const auto &[type, amount]: recipe.get_inputs()) {
        available -= town.mp.get_price(type) * amount * effective_level;
    }
    for (const auto &[type, amount]: recipe.get_outputs()) {
        available += town.mp.get_price(type) * amount * effective_level;
    }
    available *= 30;
    return available;
}

void SubsistenceFarm::pay_employees() {
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

void SubsistenceFarm::give_cargo_grain(int pop_id) {
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

void SubsistenceFarm::set_local_town(Vector2i p_town) {
    local_town = p_town;
}

Ref<Town> SubsistenceFarm::get_local_town() const {
    if (local_town == Vector2i(0, 0)) return Ref<Town>(nullptr);
    return TerminalMap::get_instance()->get_town(local_town);
}

void SubsistenceFarm::sell_cargo() {
    Ref<Town> town = TerminalMap::get_instance()->get_terminal_as<Town>(local_town);
    for (const auto& [type, __]: get_outputs()) {
        sell_type(town, type, storage[type]);
    }
    
}

void SubsistenceFarm::sell_type(Ref<Town> town, int type, int amount) {
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

double SubsistenceFarm::get_batch_size() const {
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

void SubsistenceFarm::create_recipe() {
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

void SubsistenceFarm::month_tick() {
    create_recipe();
    pay_employees();
    if (local_town != Vector2i(0, 0)) sell_cargo();
}

void SubsistenceFarm::consider_upgrade() {
    std::scoped_lock lock(m);
    if (recipe->get_employment_rate() > 0.8) {
        recipe->upgrade();
    }
}

void SubsistenceFarm::consider_degrade() {
    std::scoped_lock lock(m);
    if (recipe->get_employment_rate() < 0.5) {
        recipe->upgrade();
    }
}