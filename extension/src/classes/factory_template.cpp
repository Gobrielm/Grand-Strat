#include "factory_template.hpp"
#include "factory_local_price_controller.hpp"
#include "factory_utility/recipe.hpp"
#include "base_pop.hpp"
#include "../singletons/cargo_info.hpp"
#include "../singletons/terminal_map.hpp"
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

    ClassDB::bind_method(D_METHOD("is_hiring", "pop"), &FactoryTemplate::is_hiring);
    ClassDB::bind_method(D_METHOD("is_firing"), &FactoryTemplate::is_firing);
    ClassDB::bind_method(D_METHOD("get_wage"), &FactoryTemplate::get_wage);

    ClassDB::bind_method(D_METHOD("work_here", "pop"), &FactoryTemplate::work_here);
    ClassDB::bind_method(D_METHOD("pay_employees"), &FactoryTemplate::pay_employees);
    ClassDB::bind_method(D_METHOD("fire_employees"), &FactoryTemplate::fire_employees);

    ClassDB::bind_method(D_METHOD("day_tick"), &FactoryTemplate::day_tick);
    ClassDB::bind_method(D_METHOD("month_tick"), &FactoryTemplate::month_tick);

    ADD_SIGNAL(MethodInfo("Check_Tile_Magnitude", PropertyInfo(Variant::OBJECT, "fact"), PropertyInfo(Variant::INT, "curr_mag")));
}

FactoryTemplate::FactoryTemplate() {}


FactoryTemplate::FactoryTemplate(Vector2i new_location, int player_owner, Recipe* p_recipe) {
    initialize(new_location, player_owner, p_recipe);
}

FactoryTemplate::~FactoryTemplate() {}

void FactoryTemplate::initialize(Vector2i new_location, int player_owner, Recipe* p_recipe) {
    Broker::initialize(new_location, player_owner);
    recipe = p_recipe;
    local_pricer = memnew(FactoryLocalPriceController);
}

float FactoryTemplate::get_min_price(int type) const {
    ERR_FAIL_COND_V(get_outputs().size() != 1 || get_inputs().size() != 0, 0.0);
    return 0.0;
}

float FactoryTemplate::get_max_price(int type) const {
    ERR_FAIL_COND_V(get_outputs().size() != 0, 100.0);
    return 100.0;
}

bool FactoryTemplate::does_create(int type) const {
    std::scoped_lock lock(m);
    return get_outputs().count(type);
}

std::unordered_map<int, int> FactoryTemplate::get_outputs() const {
    return recipe->get_outputs();
}

std::unordered_map<int, int> FactoryTemplate::get_inputs() const {
    return recipe->get_inputs();
}

void FactoryTemplate::create_recipe() {
    int batch_size = get_batch_size();
    remove_inputs(batch_size);
    add_outputs(batch_size);
}

int FactoryTemplate::get_batch_size() const {
    int batch_size = get_level();
    for (auto& [type, amount]: get_inputs()) {
        batch_size = std::min((int)floor(get_cargo_amount(type) / amount), batch_size);
    }
    for (auto& [type, amount]: get_outputs()) {
        batch_size = std::min((int)floor((get_max_storage() - get_cargo_amount(type)) / amount), batch_size);
    }
    return batch_size;
}

void FactoryTemplate::remove_inputs(int batch_size) {
    for (auto& [type, amount]: get_inputs()) {
        remove_cargo(type, amount * batch_size);
    }
}

void FactoryTemplate::add_outputs(int batch_size) {
    for (auto& [type, amount]: get_outputs()) {
        add_cargo_ignore_accepts(type, amount * batch_size);
    }
}

String FactoryTemplate::get_recipe_as_string() const {
    Ref<CargoInfo> cargo_info = CargoInfo::get_instance();
    String x;
    int i = 0;
    const auto outputs = get_outputs();
    const auto inputs = get_outputs();
    for (const auto& [type, amount]: outputs) {
        x += String::num(amount) + " " + cargo_info->get_cargo_name(type);
        if (i < outputs.size() - 1) x += ", " ;
        i++;
    }
    if (inputs.size() != 0) x += " -> ";
    i = 0;
    for (const auto& [type, amount]: inputs) {
        x += String::num(amount) + " " + cargo_info->get_cargo_name(type);
        if (i < inputs.size() - 1) x += ", " ;
        i++;
    }
    return x;
}

int FactoryTemplate::get_primary_type() const {
    if (recipe->get_outputs().size() == 0) {
        return -1;
    } else {
        return (*get_outputs().begin()).first;
    }
}

void FactoryTemplate::distribute_cargo() {
    for (const auto& [type, __]: get_outputs()) {
        TradeOrder* order = get_order(type);
        if (order && order->is_sell_order()) {
            report_demand_of_brokers(type);
            distribute_from_order(order);
        }
    }
}

int FactoryTemplate::get_level() const {
    return round(recipe->get_level());
}

int FactoryTemplate::get_level_without_employment() const {
    return recipe->get_level_without_employment();
}

bool FactoryTemplate::is_max_level() const {
    if (is_primary_factory()) {
        return get_level_without_employment() == TerminalMap::get_instance()->get_cargo_value_of_tile(get_location(), get_primary_type_production());
    }

    return false;
}

int FactoryTemplate::get_cost_for_upgrade() {
    return COST_FOR_UPGRADE;
}

void FactoryTemplate::check_for_upgrade() {
    //Signal will check with cargo_values to see if it can upgrade
    if (get_inputs().size() == 0 && get_outputs().size() == 1) {
        emit_signal("Check_Tile_Magnitude", this, get_level_without_employment());
    }
}

void FactoryTemplate::upgrade() {
    int cost = get_cost_for_upgrade();
    if (get_cash() >= cost && can_factory_upgrade()) {
        remove_cash(cost);
        recipe->upgrade();
    }
}

void FactoryTemplate::admin_upgrade() {
    if (can_factory_upgrade()) {
        recipe->upgrade();
    }
    
}

bool FactoryTemplate::can_factory_upgrade() const {
    if (is_primary_factory()) {
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
    m.lock();
    income_list.push_front(change_in_cash);
    if (income_list.size() == 27) {
        income_list.pop_back();
    }
    m.unlock();
    change_in_cash = get_cash();
}

float FactoryTemplate::get_last_month_income() const {
    std::scoped_lock lock(m);
    if (income_list.size() == 0) return 0;
    return income_list.front();
}

bool FactoryTemplate::is_hiring(const BasePop* pop) const {
    return (recipe->is_pop_needed(pop));
}

bool FactoryTemplate::is_firing() const {
    return false;
}

float FactoryTemplate::get_wage() const {
    float available = 0;
    for (const auto &[type, amount]: get_inputs()) {
        available -= get_local_price(type) * amount;
    }
    for (const auto &[type, amount]: get_outputs()) {
        available += get_local_price(type) * amount;
    }
    available *= 30;

    return available / recipe->get_pops_needed_num();
}

void FactoryTemplate::work_here(BasePop* pop) {
    if (is_hiring(pop)) {
        m.lock();
        recipe->add_pop(pop);
        m.unlock();
        pop->employ(get_wage());
    }
}

void FactoryTemplate::pay_employees() {
    float wage = get_wage();
    for (BasePop* employee : recipe->get_employees()) {
        employee->pay_wage(transfer_cash(wage));
    }
}

void FactoryTemplate::fire_employees() {
    std::scoped_lock lock(m);
    recipe->fire_employees();
}

void FactoryTemplate::day_tick() {
    print_error("Never be Run, factory template day tick");
    ERR_FAIL();
}

void FactoryTemplate::month_tick() {
    update_income_array();
    pay_employees();
    if (is_firing()) {
        fire_employees();
    }
}


const int FactoryTemplate::COST_FOR_UPGRADE = 1000;
const int FactoryTemplate::DEFAULT_BATCH_SIZE = 1;