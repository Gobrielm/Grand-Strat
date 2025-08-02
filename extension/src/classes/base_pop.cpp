#include "base_pop.hpp"
#include "../singletons/terminal_map.hpp"
#include <godot_cpp/core/class_db.hpp>

void BasePop::_bind_methods() {
    ClassDB::bind_static_method(BasePop::get_class_static(), D_METHOD("get_people_per_pop"), &BasePop::get_people_per_pop);


    ClassDB::bind_method(D_METHOD("get_pop_id"), &BasePop::get_pop_id);
    ClassDB::bind_method(D_METHOD("get_home_prov_id"), &BasePop::get_home_prov_id);
    ClassDB::bind_method(D_METHOD("is_seeking_employment"), &BasePop::is_seeking_employment);
    ClassDB::bind_method(D_METHOD("pay_wage", "wage"), &BasePop::pay_wage);
    ClassDB::bind_method(D_METHOD("employ", "wage"), &BasePop::employ);
    ClassDB::bind_method(D_METHOD("fire"), &BasePop::fire);
    ClassDB::bind_method(D_METHOD("get_income"), &BasePop::get_income);
    ClassDB::bind_method(D_METHOD("is_income_acceptable", "p_income"), &BasePop::is_income_acceptable);
    ClassDB::bind_method(D_METHOD("get_expected_income"), &BasePop::get_expected_income);
    ClassDB::bind_method(D_METHOD("get_sol"), &BasePop::get_sol);
    ClassDB::bind_method(D_METHOD("buy_good", "type", "price"), &BasePop::buy_good);
    ClassDB::bind_method(D_METHOD("get_education_level"), &BasePop::get_education_level);
    ClassDB::bind_method(D_METHOD("get_wealth"), &BasePop::get_wealth);
    ClassDB::bind_method(D_METHOD("transfer_wealth"), &BasePop::transfer_wealth);
    ClassDB::bind_method(D_METHOD("get_culture"), &BasePop::get_culture);

}

std::atomic<int> BasePop::total_pops = 0;
std::unordered_map<int, float> BasePop::base_needs;
std::unordered_map<int, float> BasePop::specialities;

BasePop::BasePop(): BasePop(-1, Vector2i(0, 0), -1) {}

BasePop::BasePop(int p_home_prov_id, Vector2i p_location, Variant p_culture): pop_id(total_pops++) {
    location = p_location;
    home_prov_id = p_home_prov_id;
    culture = p_culture;
    
    wealth = INITIAL_WEALTH;
    income = 0.0;
    education_level = 0;
    for (const auto &[type, __]: base_needs) {
        internal_storage[type] = 0.0;
    }
    for (const auto &[type, __]: specialities) {
        internal_storage[type] = 0.0;
    }
}

BasePop::~BasePop() {}

String BasePop::_to_string() const {
    return "BasePop";
}

void BasePop::create_base_needs(std::unordered_map<int, float> p_base_needs) {
    for (const auto [type, amount]: p_base_needs) {
        base_needs[type] = amount;
    }
}

int BasePop::get_people_per_pop() {
    return PEOPLE_PER_POP;
}

int BasePop::get_pop_id() const {
    return pop_id;
}

int BasePop::get_home_prov_id() const {
    return home_prov_id;
}

void BasePop::set_location(Vector2i p_location) {
    location = p_location;
}
Vector2i BasePop::get_location() const {
    return location;
}

void BasePop::set_home_prov_id(int p_home_prov_id) {
    home_prov_id = p_home_prov_id;
}

bool BasePop::is_seeking_employment() const {
    if (income == 0) {
        return true;
    } else {
        //TODO
        return false;
    }
}

void BasePop::pay_wage(float wage) {
    wealth += wage;
    income = wage;
}

void BasePop::employ(int p_employement_id, float wage) {
    employement_id = p_employement_id;
    income = wage;
}

void BasePop::fire() {
    income = 0;
}

float BasePop::get_income() const {
    return income;
}

bool BasePop::is_income_acceptable(float p_income) const {
    return p_income > get_expected_income() && p_income > get_income();
}

float BasePop::get_expected_income() const {
    //TODO
    return 0;
}

float BasePop::get_sol() const {
    //TODO
    return get_income();
}

float BasePop::get_base_need(int type) const {
    return base_needs.count(type) == 1 ? base_needs[type]: 0;
}
float BasePop::get_base_want(int type) const {
    return specialities.count(type) == 1 ? specialities[type]: 0;
}

float BasePop::get_buy_price(int type, float current_price) const {
    if (get_max_storage(type) == 0) {
        return 0;
    } else if (base_needs.count(type)) {
        return get_buy_price_for_needed_good(type, current_price);
    } else {
        return get_buy_price_for_wanted_good(type, current_price);
    }
}

float BasePop::get_buy_price_for_needed_good(int type, float current_price) const {
    float available_money = income; // Save a bit

    float needed = float(get_desired(type)) / get_max_storage(type); // 0 - 1;
    if (needed == 0) return 0;

    float total_needed = needed;
    for (const auto& [other_type, amount]: base_needs) {
        if (type == other_type) continue;
        float weighted_need = float(get_desired(other_type)) / get_max_storage(other_type); // 0 - 1;
        total_needed += weighted_need;
        if (is_need_met(other_type)) break; // If need isn't met then just prioritize these goods
    }
    float price = available_money * (needed / total_needed);
    if (price > current_price) {
        return (price + current_price) / 2;
    } else {
        return price;
    }
}

float BasePop::get_buy_price_for_wanted_good(int type, float current_price) const {
    if (!are_needs_met()) {
        return 0;
    }

    float available_money = income * 0.95; // Save a bit
    
    float wanted = float(get_desired(type)) / get_max_storage(type); // 0 - 1;
    if (wanted == 0) return 0;

    float total_wanted = wanted;
    for (const auto& [other_type, __]: specialities) {
        if (type == other_type) continue;
        float weighted_want = float(get_desired(other_type)) / get_max_storage(other_type); // 0 - 1;
        total_wanted += weighted_want;
        if (!is_want_met(type)) break;
    }
    for (const auto& [other_type, __]: base_needs) {
        if (type == other_type) continue;
        float weighted_need = float(get_desired(other_type)) / get_max_storage(other_type); // 0 - 1;
        total_wanted += weighted_need;
    }

    float price = available_money * (wanted / total_wanted);
    if (price > current_price) {
        return (price + current_price) / 2;
    } else {
        return price;
    }
}

bool BasePop::are_needs_met() const {
    for (const auto &[type, amount]: internal_storage) {
        if (!(is_need_met(type))) {
            return false;
        }
    }
    return true;
}

bool BasePop::is_need_met(int type) const {
    return (internal_storage.at(type) >= get_base_need(type));
}

bool BasePop::is_want_met(int type) const {
    return (internal_storage.at(type) >= (get_base_need(type) + get_base_want(type)));
}

int BasePop::get_desired(int type) const {
    if (!internal_storage.count(type)) {
        return 0;
    }
    int amount = int(get_max_storage(type) - internal_storage.at(type));
    if ((income == 0.0 || !are_needs_met()) && !base_needs.count(type)) {
        return 0; // Don't buy if not neccessary and no job
    }
	
	return amount;
}

int BasePop::get_desired(int type, float price) const {
    int amount = std::min(int(wealth / price), int(get_max_storage(type) - internal_storage.at(type)));
    if (income == 0.0 && !base_needs.count(type)) {
        return 0; // Don't buy if not neccessary and no job
    }
	
	return amount;
}

void BasePop::buy_good(int type, int amount, float price) {
    wealth -= amount * price;
    internal_storage[type] += amount;
	if (wealth < 0) //TODO, uh-oh
        ERR_FAIL_MSG("Not enough money to buy good as pop");
}

int BasePop::get_max_storage(int type) const {
    return ceil((get_base_need(type) + get_base_want(type)) * 3.0); // Storage is based on need, so pops have 3 months without any access before bad
}

int BasePop::get_education_level() const {
    return education_level;
}

float BasePop::get_wealth() const {
    return wealth;
}

float BasePop::transfer_wealth() {
    float toReturn = get_wealth() * 0.5;
	wealth -= toReturn;
	return toReturn;
}

Variant BasePop::get_culture() const {
    return culture;
}

float BasePop::get_average_fulfillment() const {
    return std::fmin((internal_storage.at(10) / base_needs.at(10)), 1.0);
}

void BasePop::month_tick() {
    for (const auto& [type, __]: internal_storage) {
        internal_storage[type] -= (get_base_need(type) + get_base_want(type));
        if (internal_storage[type] < 0) {
            // DO something
            internal_storage[type] = 0;
        }
    }
}