#include "base_pop.hpp"
#include "../singletons/terminal_map.hpp"
#include <godot_cpp/core/class_db.hpp>

void BasePop::_bind_methods() {
    ClassDB::bind_static_method(BasePop::get_class_static(), D_METHOD("create", "p_home_prov_id", "p_culture"), &BasePop::create);
    ClassDB::bind_method(D_METHOD("initialize", "p_home_prov_id", "p_culture"), &BasePop::initialize);
    ClassDB::bind_static_method(BasePop::get_class_static(), D_METHOD("create_base_needs", "d"), &BasePop::create_base_needs);
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


    ClassDB::add_property(get_class_static(),  PropertyInfo(Variant::INT, "pop_id"), "", "get_pop_id");
    ClassDB::add_property(get_class_static(),  PropertyInfo(Variant::INT, "education_level"), "", "get_education_level");
    ClassDB::add_property(get_class_static(),  PropertyInfo(Variant::FLOAT, "wealth"), "", "get_wealth");
    ClassDB::add_property(get_class_static(),  PropertyInfo(Variant::INT, "home_prov_id"), "", "get_home_prov_id");
    ClassDB::add_property(get_class_static(),  PropertyInfo(Variant::INT, "culture"), "", "get_culture");
    ClassDB::add_property(get_class_static(),  PropertyInfo(Variant::INT, "income"), "", "get_income");
}

int BasePop::total_pops = 0;
std::mutex BasePop::m;
std::unordered_map<int, float> BasePop::base_needs;
std::unordered_map<int, float> BasePop::specialities;

BasePop::BasePop(): BasePop(-1, 0) {}

BasePop::BasePop(int p_home_prov_id, Variant p_culture) {
    home_prov_id = p_home_prov_id;
    culture = p_culture;
    m.lock();
    pop_id = total_pops++;
    m.unlock();
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

BasePop* BasePop::create(int p_home_prov_id, Variant p_culture) {
    return memnew(BasePop(p_home_prov_id, p_culture));
}

void BasePop::initialize(int p_home_prov_id, Variant p_culture) {
    home_prov_id = p_home_prov_id;
    culture = p_culture;
}

void BasePop::create_base_needs(Dictionary d) { //d<int, float>
    Array keys = d.keys();
    for (int i = 0; i < keys.size(); i++) {
        int type = keys[i];
        base_needs[type] = d[type];
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

bool BasePop::will_work_here(Ref<FactoryTemplate> fact) const {
    //Checking of suitability, eg Literacy, 
    
    if (!fact -> is_hiring(this)) {
        return false;
    } 
	float income = fact -> get_wage();
    bool val = is_income_acceptable(income);
    return val;
}

void BasePop::work_here(Ref<FactoryTemplate> work) {
    work -> work_here(this);
}

void BasePop::pay_wage(float wage) {
    wealth += wage;
}

void BasePop::employ(float wage) {
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

int BasePop::get_base_need(int type) const {
    return base_needs.count(type) ? base_needs[type]: 0;
}
int BasePop::get_base_want(int type) const {
    return specialities.count(type) ? specialities[type]: 0;
}

int BasePop::get_desired(int type) const {
    if (!internal_storage.count(type)) {
        return 0;
    }
    int amount = int(get_max_storage(type) - internal_storage.at(type));
    if (income == 0.0 && !base_needs.count(type)) {
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
    return (get_base_need(type) + get_base_want(type)) * 3; // Storage is based on need, so pops have 3 months without any access before bad
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