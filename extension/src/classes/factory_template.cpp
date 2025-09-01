#include "factory_template.hpp"
#include "factory_utility/recipe.hpp"
#include "base_pop.hpp"
#include "broker_utility/trade_interaction.hpp"
#include "../singletons/cargo_info.hpp"
#include "../singletons/terminal_map.hpp"
#include "../singletons/pop_manager.hpp"
#include "../singletons/data_collector.hpp"
#include <godot_cpp/core/class_db.hpp>
#include <algorithm>

using namespace godot;

void FactoryTemplate::_bind_methods() {
    ClassDB::bind_method(D_METHOD("get_min_price", "type"), &FactoryTemplate::get_min_price);
    ClassDB::bind_method(D_METHOD("get_max_price", "type"), &FactoryTemplate::get_max_price);
    ClassDB::bind_method(D_METHOD("does_create", "type"), &FactoryTemplate::does_create);

    ClassDB::bind_method(D_METHOD("get_recipe_as_string"), &FactoryTemplate::get_recipe_as_string);
    

    ClassDB::bind_method(D_METHOD("distribute_cargo"), &FactoryTemplate::distribute_cargo);
    ClassDB::bind_method(D_METHOD("get_level_without_employment"), &FactoryTemplate::get_level_without_employment);
    ClassDB::bind_method(D_METHOD("upgrade"), &FactoryTemplate::upgrade);
    ClassDB::bind_method(D_METHOD("admin_upgrade"), &FactoryTemplate::admin_upgrade);

    ClassDB::bind_method(D_METHOD("get_last_month_income"), &FactoryTemplate::get_last_month_income);

    ClassDB::bind_method(D_METHOD("is_firing"), &FactoryTemplate::is_firing);
    ClassDB::bind_method(D_METHOD("get_wage"), &FactoryTemplate::get_wage);

    ClassDB::bind_method(D_METHOD("pay_employees"), &FactoryTemplate::pay_employees);
    ClassDB::bind_method(D_METHOD("fire_employees"), &FactoryTemplate::fire_employees);

    ClassDB::bind_method(D_METHOD("day_tick"), &FactoryTemplate::day_tick);
    ClassDB::bind_method(D_METHOD("month_tick"), &FactoryTemplate::month_tick);
}

FactoryTemplate::FactoryTemplate() {}


FactoryTemplate::FactoryTemplate(Vector2i new_location, int player_owner, Recipe* p_recipe): Broker(new_location, player_owner) {
    recipe = p_recipe;
    if (recipe == nullptr) print_error("Factory created with null recipe at location " + new_location + ".");
    local_pricer = new LocalPriceController;
}

FactoryTemplate::~FactoryTemplate() {
    delete recipe;
}

void FactoryTemplate::initialize(Vector2i new_location, int player_owner, Recipe* p_recipe) {
    Broker::initialize(new_location, player_owner);
    recipe = p_recipe;
    local_pricer = new LocalPriceController;
}

void FactoryTemplate::create_construction_materials() {
    Ref<CargoInfo> cargo_info = CargoInfo::get_instance();
    create_construction_material(cargo_info->get_cargo_type("wood"), 100);
}

void FactoryTemplate::create_construction_material(int type, int amount) {
    add_accept(type);
    std::scoped_lock lock(m);
	max_amounts_of_construction_materials[type] = amount;
	construction_materials[type] = 0;
}

Dictionary FactoryTemplate::get_construction_materials() const {
    Dictionary d;
    std::scoped_lock lock(m);
    for (const auto& [key, val]: construction_materials) {
        d[key] = val;
    }
    return d;
}

bool FactoryTemplate::is_needed_for_construction(int type) const {
    std::scoped_lock lock(m);
    return is_needed_for_construction_unsafe(type);
}

bool FactoryTemplate::is_needed_for_construction_unsafe(int type) const {
    if (max_amounts_of_construction_materials.count(type)) {
        return max_amounts_of_construction_materials.at(type) == construction_materials.at(type);
    }
    return false;
}

int FactoryTemplate::get_amount_of_type_needed_for_construction_unsafe(int type) const {
    if (max_amounts_of_construction_materials.count(type)) {
        return max_amounts_of_construction_materials.at(type) - construction_materials.at(type);
    }
    return 0;
}

bool FactoryTemplate::is_finished_constructing() const {
    std::scoped_lock lock(m);
    if (construction_materials.size() == 0) return false; // If not constructing than not finished to avoid upgrading
    for (const auto& [type, val]: construction_materials) {
		if (max_amounts_of_construction_materials.at(type) != val) {
            return false;
        }
    }
    return true;
}

bool FactoryTemplate::is_constructing() const {
    std::scoped_lock lock(m);
    return !max_amounts_of_construction_materials.empty();
}

float FactoryTemplate::add_cargo_ignore_accepts(int type, float amount) { //Make sure
    float amount_needed_for_construction = 0;
    if (is_needed_for_construction(type)) {
        std::scoped_lock lock(m);
        amount_needed_for_construction = std::min(amount, max_amounts_of_construction_materials.at(type) - construction_materials.at(type));
        construction_materials[type] += amount_needed_for_construction;
    }
        
    float amount_stored = FixedHold::add_cargo_ignore_accepts(type, amount - amount_needed_for_construction);
    return amount_stored + amount_needed_for_construction;
}

float FactoryTemplate::get_min_price(int type) const {
    ERR_FAIL_COND_V(get_inputs().size() != 0, 0.0);
    float available = 0;
    for (const auto &[other_type, amount]: get_inputs()) {
        available -= get_local_price(other_type) * amount;
    }

    for (const auto &[other_type, amount]: get_outputs()) {
        if (type == other_type) continue;
        available += get_local_price(other_type) * amount;
    }
    
    return (available / type * -1);
}

float FactoryTemplate::get_max_price(int type) const {
    ERR_FAIL_COND_V(get_outputs().size() != 0, 100.0);
    float available = 0;
    for (const auto &[other_type, amount]: get_outputs()) {
        available += get_local_price(other_type) * amount;
    }
    for (const auto &[other_type, amount]: get_inputs()) {
        if (type == other_type) continue;
        available -= get_local_price(other_type) * amount;
    }
    return available / type;
}

bool FactoryTemplate::does_create(int type) const {
    return get_outputs().count(type);
}

bool FactoryTemplate::does_accept(int type) const {
    std::scoped_lock lock(m);
    return does_accept_unsafe(type);
}

bool FactoryTemplate::does_accept_unsafe(int type) const {
    return recipe->get_inputs().count(type) || is_needed_for_construction_unsafe(type);
}

int FactoryTemplate::get_desired_cargo(int type, float price_per) const {
    std::scoped_lock lock(m);
    return get_desired_cargo_unsafe(type, price_per);
}

int FactoryTemplate::get_desired_cargo_unsafe(int type, float price_per) const {
    if (does_accept_unsafe(type)) {
        int canGet = std::min(int(get_max_storage_unsafe(type) - get_cargo_amount_unsafe(type)), get_amount_can_buy_unsafe(price_per));
        int wanted_for_recipe = recipe->get_inputs().count(type) ? recipe->get_inputs().at(type): 0;
        int wanted_for_construction = get_amount_of_type_needed_for_construction_unsafe(type);
        return std::min((wanted_for_construction + wanted_for_recipe), canGet);
    }
    return 0;
}

std::unordered_map<int, float> FactoryTemplate::get_outputs() const {
    std::scoped_lock lock(m);
    return recipe->get_outputs();
}

std::unordered_map<int, float> FactoryTemplate::get_inputs() const {
    std::scoped_lock lock(m);
    return recipe->get_inputs();
}

void FactoryTemplate::create_recipe() {
    double batch_size = get_batch_size();
    remove_inputs(batch_size);
    add_outputs(batch_size);
}

double FactoryTemplate::get_batch_size() const {
    std::scoped_lock lock(m);
    double batch_size = recipe->get_level();
    for (auto& [type, amount]: recipe->get_inputs()) {
        batch_size = std::min(storage.at(type) / double(amount), batch_size);
    }
    for (auto& [type, amount]: recipe->get_outputs()) {
        batch_size = std::min((double(get_max_storage()) - storage.at(type)) / amount, batch_size);
    }
    return batch_size;
}

void FactoryTemplate::remove_inputs(double batch_size) {
    for (auto& [type, amount]: get_inputs()) {
        remove_cargo(type, amount * batch_size);
    }
}

void FactoryTemplate::add_outputs(double batch_size) {
    for (auto& [type, amount]: get_outputs()) {
        add_cargo_ignore_accepts(type, amount * batch_size);
        DataCollector::get_instance()->add_supply(type, amount * batch_size);
    }
}

String FactoryTemplate::get_recipe_as_string() const {
    Ref<CargoInfo> cargo_info = CargoInfo::get_instance();
    String x;
    int i = 0;
    const auto outputs = get_outputs();
    const auto inputs = get_inputs();
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

int FactoryTemplate::get_primary_type() const {

    if (get_outputs().size() == 0) {
        return -1;
    } else {
        return (*get_outputs().begin()).first;
    }
}

void FactoryTemplate::add_monthly_demand_across_broad_market() {
    auto broker_ids = get_broker_ids_in_broad_market();
    for (const auto& [type, __]: get_outputs()) {
        for (const auto& id: broker_ids) {
            survey_broker_market(type, id);
        }
    }
}

void FactoryTemplate::distribute_cargo() {
    for (const auto& [type, __]: get_outputs()) {
        distribute_type(type);
    }
}

double FactoryTemplate::get_level() const {
    std::scoped_lock lock(m);
    return recipe->get_level();
}

int FactoryTemplate::get_level_without_employment() const {
    std::scoped_lock lock(m);
    return recipe->get_level_without_employment();
}

bool FactoryTemplate::is_max_level() const {
    if (is_primary_factory()) {
        return get_level_without_employment() == TerminalMap::get_instance()->get_cargo_value_of_tile(get_location(), get_primary_type_production());
    }

    return false;
}

void FactoryTemplate::upgrade() {
    if (can_factory_upgrade()) {
        create_construction_materials();
    }
}

void FactoryTemplate::admin_upgrade() {
    if (can_factory_upgrade()) {
        std::scoped_lock lock(m);
        recipe->upgrade();
    }
}

void FactoryTemplate::finish_upgrade() {
    print_line("Finished Upgrade.");
    std::scoped_lock lock(m);
    max_amounts_of_construction_materials.clear();
    construction_materials.clear();
    recipe->upgrade();
}

bool FactoryTemplate::can_factory_upgrade() const {
    if (is_primary_factory() && !is_constructing()) {
        return TerminalMap::get_instance()->get_cargo_value_of_tile(get_location(), get_primary_type_production()) > get_level_without_employment();
    }
    return true;
}

bool FactoryTemplate::is_primary_factory() const {
    return get_outputs().size() == 1 && CargoInfo::get_instance()->is_cargo_primary(get_primary_type_production());
}

int FactoryTemplate::get_primary_type_production() const {
    return (*get_outputs().begin()).first;
}

void FactoryTemplate::update_income_array() {
    {
        std::scoped_lock lock(m);
        income_list.push_front(change_in_cash);
        if (income_list.size() == 27) {
            income_list.pop_back();
        }
    }
    
    change_in_cash = get_cash();
}

float FactoryTemplate::get_last_month_income() const {
    std::scoped_lock lock(m);
    if (income_list.size() == 0) return 0;
    return income_list.front();
}

bool FactoryTemplate::is_hiring() const {
    return get_theoretical_gross_profit_unsafe() > 0;
}

bool FactoryTemplate::is_hiring(PopTypes pop_type) const {
    std::scoped_lock lock(m);

    return recipe->is_pop_type_needed(pop_type) && is_hiring_tracker; // TODO: Check requirements
}

bool FactoryTemplate::is_firing() const {
    if (get_theoretical_gross_profit() < 0) {
        return true;
    }
    return false;
}

float FactoryTemplate::get_wage() const {
    std::scoped_lock lock(m);
    int pops_needed_num = recipe->get_pops_needed_num();
    if (pops_needed_num == 0) return 0;
    float gross_profit = std::min(float(get_theoretical_gross_profit_unsafe()), get_cash_unsafe());
    return (gross_profit) / pops_needed_num;
}

float FactoryTemplate::get_wage_unsafe() const {
    float gross_profit = std::min(float(get_theoretical_gross_profit_unsafe()), get_cash_unsafe());
    int pops_needed_num = recipe->get_pops_needed_num();
    if (!pops_needed_num) return 0;
    
    return (gross_profit) / pops_needed_num;
}

float FactoryTemplate::get_theoretical_gross_profit() const {
    float available = 0;
    
    for (const auto &[type, amount]: get_inputs()) {
        available -= get_local_price(type) * amount * std::max(get_level(), 1.0); // Always assume that the business will pay according to the first level, so hiring wont bug out
    }
    for (const auto &[type, amount]: get_outputs()) {
        available += get_local_price(type) * amount * std::max(get_level(), 1.0);
    }
    available *= 30;
    return available;
}

float FactoryTemplate::get_theoretical_gross_profit_unsafe() const {
    float available = 0;
    
    for (const auto &[type, amount]: recipe->get_inputs()) {
        available -= get_local_price_unsafe(type) * amount * std::max(recipe->get_level(), 1.0); // Always assume that the business will pay according to the first level, so hiring wont bug out
    }
    for (const auto &[type, amount]: recipe->get_outputs()) {
        available += get_local_price_unsafe(type) * amount * std::max(recipe->get_level(), 1.0);
    }
    available *= 30;
    return available;
}

float FactoryTemplate::get_real_gross_profit(int months_to_average) const {
    ERR_FAIL_COND_V_EDMSG(months_to_average <= 0, 0, "Cannot average over a 0 or negitive amount of months");
    float total = 0;
    int i = 1;
    for (auto it = income_list.begin(); it != income_list.end(); it++) {
        total += (*it);
        if (++i > months_to_average) {
            break;
        }       
    }
    return total / i;
}

void FactoryTemplate::employ_pop(BasePop* pop, std::shared_mutex &pop_lock, PopTypes pop_type) {
    if (is_hiring(pop_type)) {
        float wage;
        {
            std::scoped_lock lock(m);
            recipe->add_pop(pop_type, pop->get_pop_id());
            wage = get_wage_unsafe();
        }
        
        {
            std::scoped_lock lock(pop_lock);
            pop->employ(terminal_id, wage);
            pop->set_location(get_location());
        }
        
    }
}

void FactoryTemplate::pay_employees() {
    auto pop_manager = PopManager::get_instance();
    float wage = get_wage();
    std::unordered_map<int, PopTypes> employees;
    {
        std::scoped_lock lock(m);
        employees = recipe->get_employee_ids();
    }

    for (const auto& [pop_id, __] : employees) {
        pop_manager->pay_pop(pop_id, transfer_cash(wage));
    }
}

void FactoryTemplate::fire_employees() {
    auto pop_manager = PopManager::get_instance();
    std::vector<int> pops_to_fire;
    {
        std::scoped_lock lock(m);
        pops_to_fire = recipe->fire_employees_and_get_vector();
    }
    for (const auto& pop_id : pops_to_fire) {
        pop_manager->fire_pop(pop_id);
    }
}

void FactoryTemplate::day_tick() {
    print_error("Never be Run, factory template day tick");
    ERR_FAIL();
}

void FactoryTemplate::month_tick() { 
    add_monthly_demand_across_broad_market();
    update_income_array();
    pay_employees();
    if (is_hiring()) {
        is_hiring_tracker = true;
    } else {
        is_hiring_tracker = false;
    }
    if (is_firing()) {
        fire_employees();
    }
    if (is_finished_constructing()) {
        finish_upgrade();
    }
}


const int FactoryTemplate::DEFAULT_BATCH_SIZE = 1;