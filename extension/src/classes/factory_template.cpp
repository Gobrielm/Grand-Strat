#include "factory_template.hpp"
#include "base_pop.hpp"
#include <godot_cpp/core/class_db.hpp>
#include <algorithm>

using namespace godot;

void FactoryTemplate::_bind_methods() {
    ClassDB::bind_static_method(get_class_static(), D_METHOD("create", "new_location", "player_owner", "new_inputs", "new_outputs"), &FactoryTemplate::create);
    ClassDB::bind_method(D_METHOD("initialize", "new_location", "player_owner", "new_inputs", "new_outputs"), &FactoryTemplate::initialize);

    ClassDB::bind_method(D_METHOD("get_min_price", "type"), &FactoryTemplate::get_min_price);
    ClassDB::bind_method(D_METHOD("get_max_price", "type"), &FactoryTemplate::get_max_price);
    ClassDB::bind_method(D_METHOD("get_monthly_demand", "type"), &FactoryTemplate::get_monthly_demand);
    ClassDB::bind_method(D_METHOD("get_monthly_supply", "type"), &FactoryTemplate::get_monthly_supply);
    ClassDB::bind_method(D_METHOD("does_create", "type"), &FactoryTemplate::does_create);

    ClassDB::bind_method(D_METHOD("distribute_cargo"), &FactoryTemplate::distribute_cargo);
    ClassDB::bind_method(D_METHOD("get_level"), &FactoryTemplate::get_level);
    ClassDB::bind_method(D_METHOD("upgrade"), &FactoryTemplate::upgrade);
    ClassDB::bind_method(D_METHOD("admin_upgrade"), &FactoryTemplate::admin_upgrade);

    ClassDB::bind_method(D_METHOD("get_last_month_income"), &FactoryTemplate::get_last_month_income);

    ClassDB::bind_method(D_METHOD("get_employement"), &FactoryTemplate::get_employement);
    ClassDB::bind_method(D_METHOD("is_hiring"), &FactoryTemplate::is_hiring);
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


FactoryTemplate::FactoryTemplate(Vector2i new_location, int player_owner, Dictionary new_inputs, Dictionary new_outputs) {
    initialize(new_location, player_owner, new_inputs, new_outputs);
}

FactoryTemplate::~FactoryTemplate() {}

Terminal* FactoryTemplate::create(Vector2i new_location, int player_owner, Dictionary new_inputs, Dictionary new_outputs) {
    return memnew(FactoryTemplate(new_location, player_owner, new_inputs, new_outputs));
}
void FactoryTemplate::initialize(Vector2i new_location, int player_owner, Dictionary new_inputs, Dictionary new_outputs) {
    Broker::initialize(new_location, player_owner);
    Array input_keys = new_inputs.keys();
    for (int i = 0; i < input_keys.size(); i++) {
        int type = input_keys[i];
        inputs[type] = new_inputs[type];
    }
    Array output_keys = new_outputs.keys();
    for (int i = 0; i < output_keys.size(); i++) {
        int type = output_keys[i];
        inputs[type] = new_outputs[type];
    }
    local_pricer = memnew(LocalPriceController(inputs, outputs));
    pops_needed = 1;
}

float FactoryTemplate::get_min_price(int type) const {
    ERR_FAIL_COND_V(outputs.size() != 1 || inputs.size() != 0, 0.0);
    return 0.0;
}

float FactoryTemplate::get_max_price(int type) const {
    ERR_FAIL_COND_V(outputs.size() != 0, 100.0);
    return 100.0;
}

int FactoryTemplate::get_monthly_demand(int type) const {
    //TODO: ClockSingleton::get_instance()->get_days_in_current_month()
    return inputs.at(type) * 30 * level;
}

int FactoryTemplate::get_monthly_supply(int type) const {
    //TODO: ClockSingleton::get_instance()->get_days_in_current_month()
    return outputs.at(type) * 30 * level;
}

bool FactoryTemplate::does_create(int type) const {
    return outputs.count(type);
}

void FactoryTemplate::create_recipe() {
    int batch_size = get_batch_size();
    remove_inputs(batch_size);
    add_outputs(batch_size);
}

int FactoryTemplate::get_batch_size() const {
    int batch_size = get_level();
    for (auto& [type, amount]: inputs) {
        batch_size = std::min((int)floor(get_cargo_amount(type) / amount), batch_size);
    }
    for (auto& [type, amount]: outputs) {
        batch_size = std::min((int)floor(get_max_storage() - get_cargo_amount(type) / amount), batch_size);
    }
    return batch_size;
}

void FactoryTemplate::remove_inputs(int batch_size) {
    for (auto& [type, amount]: inputs) {
        remove_cargo(type, amount * batch_size);
    }
}

void FactoryTemplate::add_outputs(int batch_size) {
    for (auto& [type, amount]: outputs) {
        add_cargo_ignore_accepts(type, amount * batch_size);
    }
}

void FactoryTemplate::distribute_cargo() {
    for (auto& [type, amount]: outputs) {
        TradeOrder* order = get_order(type);
        if (order && order->is_sell_order()) {
            distribute_from_order(order);
        }
    }
}

int FactoryTemplate::get_level() const {
    //TODO: Employment
	//var employment: int = get_employement()
	//if employment == 0:
	//  return 0
	//return round(level * employment / employment_total)
    return level;
}

int FactoryTemplate::get_cost_for_upgrade() const {
    return COST_FOR_UPGRADE;
}

void FactoryTemplate::check_for_upgrade() {
    //Signal will check with cargo_values to see if it can upgrade
    if (inputs.size() == 0 && outputs.size() == 1) {
        emit_signal("Check_Tile_Magnitude", this, level);
    }
}

void FactoryTemplate::upgrade() {
    int cost = get_cost_for_upgrade();
    if (get_cash() >= cost) {
        remove_cash(cost);
        level++;
        pops_needed = level;
    }
}

void FactoryTemplate::admin_upgrade() {
    //Make sure that thing calling does check
    level++;
    pops_needed = level;
}

void FactoryTemplate::update_income_array() {
    income_list.push_front(change_in_cash);
    if (income_list.size() == 27) {
        income_list.pop_back();
    }
    change_in_cash = get_cash();
}

float FactoryTemplate::get_last_month_income() const {
    if (income_list.size() == 0) return 0;
    return income_list.front();
}

int FactoryTemplate::get_employement() const {
    return employees.size();
}

bool FactoryTemplate::is_hiring() const {
    return (pops_needed - get_employement() >= 0) && (get_last_month_income() * 0.9 > 0);
}

bool FactoryTemplate::is_firing() const {
    return get_employement() != 0 && get_last_month_income() < 0;
}

float FactoryTemplate::get_wage() const {
    float available = get_last_month_income() * 0.9f;
    return available / pops_needed;
}

void FactoryTemplate::work_here(BasePop* pop) {
    if (is_hiring()) {
        int index = employees.empty() ? 0 : rand() % employees.size();
        employees.insert(employees.begin() + index, pop);
        pop->employ(get_wage());
    }
}

void FactoryTemplate::pay_employees() {
    float wage = get_wage();
    for (BasePop* employee : employees) {
        employee->pay_wage(transfer_cash(wage));
    }
}

void FactoryTemplate::fire_employees() {
    int fired = 0;
    int to_fire = std::max((int)(employees.size() * 0.1), 100);
    while (fired < to_fire && !employees.empty()) {
        int rand_index = rand() % employees.size();
        BasePop* pop = employees[rand_index];
        employees.erase(employees.begin() + rand_index);
        pop->fire();
        fired++;
    }
}

void FactoryTemplate::day_tick() {
    print_line("Never be Run");
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