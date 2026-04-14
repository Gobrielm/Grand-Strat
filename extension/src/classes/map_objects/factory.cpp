#include "factory.hpp"
#include <classes/factory_utility/recipe.hpp>
#include <classes/base_pop.hpp>

#include "broker_utility/trade_interaction.hpp"
#include "../singletons/cargo_info.hpp"
#include "../singletons/terminal_map.hpp"
#include "../singletons/pop_manager.hpp"
#include "../singletons/data_collector.hpp"
#include <godot_cpp/core/class_db.hpp>
#include <algorithm>


void Factory::_bind_methods() {
    ClassDB::bind_method(D_METHOD("day_tick"), &Factory::day_tick);
    ClassDB::bind_method(D_METHOD("month_tick"), &Factory::month_tick);
}

Factory::~Factory() {

}

Factory::Factory(std::pair<int, int> p_position, int p_owner, Recipe p_recipe): position(p_position, BuildingType::FACTORY) {
    owner = OwnerComponent(p_owner);
    recipe = p_recipe;
    lpc = LocalPriceController();
}

void Factory::create_construction_materials() {
    Ref<CargoInfo> cargo_info = CargoInfo::get_instance();
    construction.add_construction_material(cargo_info->get_cargo_type("wood"), 100);
}

float Factory::get_min_price(int type, std::optional<Town&> town = std::nullopt) const {
    ERR_FAIL_COND_V(recipe.get_output(type) == 0, 0.0);
    float available = 0;
    const float bias_term = 1.03;
    for (const auto &[other_type, amount]: recipe.get_inputs()) {
        float price = town.value().mp.get_price(other_type) ? town.has_value(): orders.at(type)->get_limit_price();
        available -= price * amount * bias_term;
    }

    for (const auto &[other_type, amount]: recipe.get_outputs()) {
        if (type == other_type) continue;
        float price = town.value().mp.get_price(other_type) ? town.has_value(): orders.at(type)->get_limit_price();
        available += price * amount;
    }
    
    return std::max(available / recipe.get_output(type) * -1, 0.0f);
}

float Factory::get_max_price(int type, std::optional<Town&> town = std::nullopt) const {
    ERR_FAIL_COND_V(recipe.get_input(type) == 0, 1000.0);

    float available = 0;
    const float bias_term = 1.03;

    for (const auto &[other_type, amount]: recipe.get_inputs()) {
        if (type == other_type) continue;
        float price = town.value().mp.get_price(other_type) ? town.has_value(): orders.at(type)->get_limit_price();
        available -= price * amount;
    }
    for (const auto &[other_type, amount]: recipe.get_outputs()) {
        float price = town.value().mp.get_price(other_type) ? town.has_value(): orders.at(type)->get_limit_price();
        available += price * amount * bias_term;
    }

    return std::min(available / recipe.get_input(type), 0.0f);
}

bool Factory::does_create(int type) const {
    return recipe.get_output(type) != 0;
}

bool Factory::does_accept(int type) const {
    return recipe.get_input(type) != 0 || construction.is_needed_for_construction(type);
}

int Factory::get_desired_cargo(int type, float price_per) {
    if (does_accept(type)) {

        float local_price = lpc.get_local_price(type) + 0.00001f;
        int canGet = int(std::min(storage.get_desired_cargo(type, price_per), capital.get_cash() / local_price));

        int wanted_for_recipe = recipe.get_input(type);
        int wanted_for_construction = construction.get_amount_of_type_needed_for_construction(type);

        return std::min((wanted_for_construction + wanted_for_recipe), canGet);
    }
    return 0;
}


/// @brief TODO: Add other factors
int Factory::get_desired_cargo_to_sell(int type) {
    if (recipe.get_output(type) != 0) {
        return storage.get_amount(type);
    }
    return 0;
}

void Factory::create_recipe() {
    double batch_size = get_batch_size();
    if (batch_size <= 0) return;
    remove_inputs(batch_size);
    add_outputs(batch_size);
}

double Factory::get_batch_size() {
    double batch_size = recipe.get_level();
    for (auto& [type, amount]: recipe.get_inputs()) {
        batch_size = std::min(storage.get_amount(type) / double(amount), batch_size);
    }
    for (auto& [type, amount]: recipe.get_outputs()) {
        batch_size = std::min((double(storage.MAX_STORAGE) - storage.get_amount(type)) / amount, batch_size);
    }
    return batch_size;
}

void Factory::remove_inputs(double batch_size) {
    for (auto& [type, amount]: recipe.get_inputs()) {
        storage.remove_cargo(type, amount * batch_size);
    }
}

void Factory::add_outputs(double batch_size) {
    for (auto& [type, amount]: recipe.get_outputs()) {
        storage.add_cargo(type, amount * batch_size);
        DataCollector::get_instance()->add_supply(type, amount * batch_size);
    }
}

String Factory::get_recipe_as_string() const {
    Ref<CargoInfo> cargo_info = CargoInfo::get_instance();
    String x;
    int i = 0;
    const auto& outputs = recipe.get_outputs();
    const auto& inputs = recipe.get_inputs();
    for (const auto& [type, amount]: inputs) {
        x += String::num(amount) + " " + cargo_info->get_cargo_name(type);
        if (i < inputs.size() - 1) x += ", " ;
        i++;
    }
    if (inputs.size() != 0) x += " -> ";
    i = 0;
    for (const auto& [type, amount]: outputs) {
        x += String::num(amount) + " " + cargo_info->get_cargo_name(type);
        if (i < outputs.size() - 1) x += ", " ;
        i++;
    }
    return x;
}

int Factory::get_primary_type() const {

    if (recipe.get_outputs().size() == 0) {
        return -1;
    } else {
        int primary_type = -1;
        int largest = 0;
        for (const auto& [type, amt]: recipe.get_outputs()) {
            if (amt > largest) {
                primary_type = type;
            }
        }
        return primary_type;
    }
}

void Factory::upgrade() {
    create_construction_materials();
}

void Factory::admin_upgrade() {
    recipe.upgrade();
}

void Factory::finish_upgrade() {
    print_line("Finished Upgrade.");
    construction.finish_construction();
    recipe.upgrade();
}

bool Factory::is_primary_factory() const {
    return recipe.get_outputs().size() == 1 && CargoInfo::get_instance()->is_cargo_primary(get_primary_type());
}

float Factory::get_last_month_income() const {
    return capital.get_recent_change();
}

bool Factory::is_hiring() const {
    return get_theoretical_gross_profit() > 0;
}

bool Factory::is_hiring(PopTypes pop_type) const {
    return recipe.is_pop_type_needed(pop_type);
}

bool Factory::is_firing() const {
    if (get_theoretical_gross_profit() < 0) {
        return true;
    }
    return false;
}

float Factory::get_wage() const {
    float gross_profit = std::min(float(get_theoretical_gross_profit()), capital.get_cash());
    int pops_needed_num = recipe.get_pops_needed_num();
    if (!pops_needed_num) return 0;
    return (gross_profit) / pops_needed_num;
}

float Factory::get_theoretical_gross_profit() const {
    float available = 0;
    double level = recipe.get_level();
    
    for (const auto &[type, amount]: recipe.get_inputs()) {
        available -= lpc.get_local_price(type) * amount * std::max(level, 1.0); // Always assume that the business will pay according to the first level, so hiring wont bug out
    }
    for (const auto &[type, amount]: recipe.get_outputs()) {
        available += lpc.get_local_price(type) * amount * std::max(level, 1.0);
    }
    available *= 30;
    return available;
}

float Factory::get_real_gross_profit(int months_to_average) const {
    ERR_FAIL_COND_V_EDMSG(months_to_average <= 0, 0, "Cannot average over a 0 or negitive amount of months");
    const auto& cash_history = capital.get_cash_history();

    float total = 0;
    float last = capital.get_cash();
    int months = 1;

    // Start one from back since get_cash is same
    for (auto it = std::next(cash_history.end(), -2); it != std::next(cash_history.begin(), -1); it--) {
        float income = last - *it; // last should be bigger
        total += income;
        if (++months > months_to_average) {
            break;
        }       
        last = *it;
    }
    return total / months;
}

void Factory::employ_pop(BasePop& pop) {
    PopTypes pop_type = pop.get_type();
    if (is_hiring(pop_type)) {
        float wage;
        recipe.add_pop(pop_type, pop.get_pop_id());
        wage = get_wage();
        
        pop.employ(position.building_id, wage);
        pop.set_location(position.get_position_vector2i());
    
    }
}

void Factory::day_tick() {
    print_error("Never be Run, factory template day tick");
    ERR_FAIL();
}

void Factory::month_tick() { 
    capital.update_cash_history();
    if (is_firing()) {
        recipe.queue_employees_to_be_fired();
    }
    if (construction.is_finished_constructing()) {
        finish_upgrade();
    }
    
    create_recipe();
}

void Factory::adjust_trade_orders(Town& town) {
    for (const auto& [type, amt]: recipe.get_inputs()) {
        if (!orders.count(type)) {
            orders[type] = std::make_shared<TradeOrder>(position.building_id, type, amt, true, town.mp.get_price(type), get_max_price(type, town));
            town.mp.add_order(orders[type]);
        }

        float price = orders[type]->get_limit_price();
        orders[type]->set_max_price(get_max_price(type));
        if (last_month_storage.get_amount(type) < amt) {
            float new_price = std::min(get_max_price(type), price + 0.05f);
            orders[type]->set_price(new_price);
        } else {
            orders[type]->set_price(price -= 0.05f);
        }
    }
    for (const auto& [type, amt]: recipe.get_outputs()) {
        if (!orders.count(type)) {
            orders[type] = std::make_shared<TradeOrder>(position.building_id, type, amt, false, town.mp.get_price(type), get_min_price(type, town));
            town.mp.add_order(orders[type]);
        }

        float price = orders[type]->get_limit_price();
        orders[type]->set_max_price(get_min_price(type));
        if (last_month_storage.get_amount(type) > 1) {
            float new_price = std::max(get_min_price(type), price - 0.05f);
            orders[type]->set_price(new_price);
        } else {
            orders[type]->set_price(price += 0.05f);
        }
    }
    last_month_storage = storage;
}