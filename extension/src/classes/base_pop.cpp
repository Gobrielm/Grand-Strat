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
    ClassDB::bind_method(D_METHOD("get_desired", "type", "price"), &BasePop::get_desired);
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
    TerminalMap::get_instance() -> lock(fact -> get_location());
    //Checking of suitability, eg Literacy, 
    
    if (!fact -> is_hiring()) {
        TerminalMap::get_instance() -> unlock(fact -> get_location());
        return false;
    } 
	float income = fact -> get_wage();
    TerminalMap::get_instance() -> unlock(fact -> get_location());
    bool val = is_income_acceptable(income);
    return val;
}

void BasePop::work_here(Ref<FactoryTemplate> work) {
    TerminalMap::get_instance() -> lock(work -> get_location());
    work -> work_here(this);
    TerminalMap::get_instance() -> unlock(work -> get_location());
    UtilityFunctions::print("I started working");
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

float BasePop::get_desired(int type, float price) const {
    float amount = 0;
	if (base_needs.count(type)) amount += base_needs[type];

	if (specialities.count(type)) amount += specialities[type];
	
    if (amount * price < wealth) {
        return amount;
    }
	return 0;
}

void BasePop::buy_good(float amount, float price) {
    wealth -= amount * price;
	if (wealth < 0) //TODO, uh-oh
        throw std::invalid_argument("Not enough money");
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