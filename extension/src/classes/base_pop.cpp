#include "base_pop.hpp"
#include "../singletons/terminal_map.hpp"
#include "../singletons/cargo_info.hpp"
#include <godot_cpp/core/class_db.hpp>
#include <sstream>
#include <fstream>

void BasePop::_bind_methods() {
    ClassDB::bind_method(D_METHOD("get_pop_id"), &BasePop::get_pop_id);
    ClassDB::bind_method(D_METHOD("get_home_prov_id"), &BasePop::get_home_prov_id);
    ClassDB::bind_method(D_METHOD("is_seeking_employment"), &BasePop::is_seeking_employment);
    ClassDB::bind_method(D_METHOD("pay_wage", "wage"), &BasePop::pay_wage);
    ClassDB::bind_method(D_METHOD("employ", "wage"), &BasePop::employ);
    ClassDB::bind_method(D_METHOD("fire"), &BasePop::fire);
    ClassDB::bind_method(D_METHOD("get_income"), &BasePop::get_income);
    ClassDB::bind_method(D_METHOD("get_sol"), &BasePop::get_sol);
    ClassDB::bind_method(D_METHOD("buy_good", "type", "price"), &BasePop::buy_good);
    ClassDB::bind_method(D_METHOD("get_education_level"), &BasePop::get_education_level);
    ClassDB::bind_method(D_METHOD("get_wealth"), &BasePop::get_wealth);
    ClassDB::bind_method(D_METHOD("transfer_wealth"), &BasePop::transfer_wealth);
    ClassDB::bind_method(D_METHOD("get_culture"), &BasePop::get_culture);

}

std::atomic<int> BasePop::total_pops = 0;

std::unordered_map<PopTypes, int> BasePop::PEOPLE_PER_POP = {
    {rural, 1000},
    {town, 1000},
    {peasant, 1000},
    {none, 0}
};

std::unordered_map<PopTypes, int> BasePop::INITIAL_WEALTH = {
    {rural, 1000},
    {town, 1000},
    {peasant, 1000},
    {none, 0}
};

std::unordered_map<PopTypes, std::unordered_map<int, float>> BasePop::base_needs;

std::unordered_map<PopTypes, std::unordered_map<int, float>> BasePop::specialities;

BasePop* BasePop::create_rural_pop(int p_home_prov_id, Vector2i p_location, Variant p_culture) {
    return memnew(BasePop(p_home_prov_id, p_location, p_culture, rural));
}
BasePop* BasePop::create_peasant_pop(int p_home_prov_id, Vector2i p_location, Variant p_culture) {
    return memnew(BasePop(p_home_prov_id, p_location, p_culture, peasant));
}
BasePop* BasePop::create_town_pop(int p_home_prov_id, Vector2i p_location, Variant p_culture) {
    return memnew(BasePop(p_home_prov_id, p_location, p_culture, town));
}

BasePop::BasePop(): BasePop(-1, Vector2i(0, 0), -1, none) {}

BasePop::BasePop(int p_home_prov_id, Vector2i p_location, Variant p_culture, PopTypes p_pop_type): pop_id(total_pops++) {
    location = p_location;
    home_prov_id = p_home_prov_id;
    culture = p_culture;
    pop_type = p_pop_type;
    
    wealth = INITIAL_WEALTH.at(pop_type);
    income = 0.0;
    education_level = 0;
    reset_and_fill_storage();
}

BasePop::~BasePop() {
    ERR_FAIL_COND_MSG(employement_id != -2, "Destroyed pop that was employed");
}

String BasePop::_to_string() const {
    return "BasePop";
}

void BasePop::create_base_needs() {
    base_needs = create_needs("pop_needs.xlsx");
}
void BasePop::create_base_wants() {
    specialities = create_needs("pop_wants.xlsx");
}

std::unordered_map<PopTypes, std::unordered_map<int, float>> BasePop::create_needs(std::string file_name) {
    std::unordered_map<PopTypes, std::unordered_map<int, float>> needs_map;

    std::unordered_map<std::string, PopTypes> pop_type_map = {
        {"peasant", peasant},
        {"rural", rural},
        {"town", town},
        {"none", none}
    };

    std::ifstream file(file_name);
    if (!file.is_open()) {
        std::cerr << "Failed to open " << file_name << "\n";
        return needs_map;
    }

    Ref<CargoInfo> cargo_info = CargoInfo::get_instance();
    std::string line;

    while (std::getline(file, line)) {
        // Remove trailing commas/whitespace
        line.erase(std::remove_if(line.begin(), line.end(), ::isspace), line.end());

        if (line.empty()) continue;

        std::stringstream ss(line);
        std::string token;

        // First token is the pop type string
        if (!std::getline(ss, token, ',')) continue;
        if (pop_type_map.find(token) == pop_type_map.end()) {
            std::cerr << "Unknown pop type: " << token << "\n";
            continue;
        }
        PopTypes pop_type = pop_type_map[token];

        // Read cargo-name/value pairs
        while (true) {
            std::string cargo_name;
            if (!std::getline(ss, cargo_name, ',')) break;
            if (cargo_name.empty()) break;

            std::string amount_str;
            if (!std::getline(ss, amount_str, ',')) break;

            float amount = std::stof(amount_str);
            int cargo_id = cargo_info->get_cargo_type(cargo_name.c_str());
            needs_map[pop_type][cargo_id] = amount;
        }
    }

    return needs_map;
}

int BasePop::get_people_per_pop(PopTypes pop_type) {
    return PEOPLE_PER_POP.at(pop_type);
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

PopTypes BasePop::get_type() const {
    return pop_type;
}

float BasePop::get_base_need(PopTypes pop_type, int type) {
    return base_needs[pop_type][type];
}
float BasePop::get_base_want(PopTypes pop_type, int type) {
    return specialities[pop_type][type];
}

std::unordered_map<int, float> BasePop::get_base_needs(PopTypes pop_type) {
    return base_needs[pop_type];
}

std::unordered_map<int, float> BasePop::get_base_wants(PopTypes pop_type) {
    return specialities[pop_type];
}

std::unordered_map<int, float> BasePop::get_base_needs() const {
    return base_needs.at(pop_type);
}
std::unordered_map<int, float> BasePop::get_base_wants() const {
    return specialities.at(pop_type);
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

bool BasePop::is_unemployed() const {
    return employement_id == -2 && income == 0;
}

void BasePop::pay_wage(float wage) {
    if (std::isnan(wage)) {
        ERR_FAIL_MSG("NaN wage detected");
    }
    
    wealth += wage;
    income = wage;
}

void BasePop::employ(int p_employement_id, float wage) {
    employement_id = p_employement_id;
    income = wage;
}

void BasePop::fire() {
    income = 0;
    employement_id = -2;
}

float BasePop::get_income() const {
    return income;
}

bool BasePop::is_wage_acceptable(float p_wage) const {
    return p_wage > income;
}

float BasePop::get_expected_income(std::unordered_map<int, float> current_prices) const { // TODO: For now only care about grain
    float exp_income = 0;
    for (const auto& [type, amount]: get_base_needs()) {
        exp_income += current_prices.at(type) * amount;
    }
    return exp_income;
}

float BasePop::get_sol() const {
    //TODO
    return get_income();
}

bool BasePop::is_starving() const {
    return months_starving != 0;
}

bool BasePop::is_in_mild_starvation() const {
    return months_starving > 0 && months_starving <= 2;
}

bool BasePop::is_in_medium_starvation() const {
    return months_starving > 2 && months_starving <= 5;
}

bool BasePop::is_in_high_starvation() const {
    return months_starving > 5;
}

float BasePop::get_base_need(int type) const {
    return get_base_needs().count(type) == 1 ? get_base_needs()[type]: 0;
}
float BasePop::get_base_want(int type) const {
    return get_base_wants().count(type) == 1 ? get_base_wants()[type]: 0;
}

float BasePop::get_buy_price(int type, float current_price) const {
    if (get_base_need(type) > 0) {
        return get_buy_price_for_needed_good(type, current_price);
    } else if (get_base_want(type) > 0) {
        return get_buy_price_for_wanted_good(type, current_price);
    }
    return 0;
}

float BasePop::get_buy_price_for_needed_good(int type, float current_price) const {
    float needed = float(get_desired(type)) / get_max_storage(type); // 0 - 1;
    if (needed == 0) return 0;

    float mult = (needed == 1) ? (1 + ((rand() % 5) / 100.0)): 1;
    float available_money = std::min(std::max(income, current_price * mult), wealth); // Highest will go

    float price = available_money;
    if (price > current_price) {
        return (price + current_price) / 2;
    } else {
        return price;
    }
}

float BasePop::get_buy_price_for_wanted_good(int type, float current_price) const {
    if (!are_needs_met() || income == 0) {
        return 0;
    }

    float available_money = income; // Save a bit
    
    float wanted = float(get_desired(type)) / get_max_storage(type); // 0 - 1;
    if (wanted == 0) return 0;

    float total_wanted = wanted;
    for (const auto& [other_type, __]: get_base_wants()) {
        if (type == other_type) continue;
        float weighted_want = float(get_desired(other_type)) / get_max_storage(other_type); // 0 - 1;
        total_wanted += weighted_want;
        if (!is_want_met(type)) break;
    }
    for (const auto& [other_type, __]: get_base_needs()) {
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
    int amount = std::max(int(get_max_storage(type) - internal_storage.at(type)), 0);
    if (income == 0 && !get_base_needs().count(type)) { // Don't buy if not neccessary and no job
        return 0; 
    }
	
	return amount;
}

int BasePop::get_desired(int type, float price) const {
    int amount = std::max(std::min(int(wealth / price), int(get_max_storage(type) - internal_storage.at(type))), 0);
    if (income == 0.0 && !get_base_needs().count(type)) {
        return 0; // Don't buy if not neccessary and no job
    }
	
	return amount;
}

void BasePop::buy_good(int type, int amount, float price) {
    if (std::isnan(price)) {
        ERR_FAIL_MSG("NaN in buy_good inputs");
    }
    if (amount < 0) {
        ERR_FAIL_MSG("Amount is negitive");
    }
    wealth -= amount * price;
    internal_storage[type] += amount;
	if (wealth < 0) { //TODO, uh-oh
        ERR_FAIL_MSG("Not enough money to buy good as pop");
    }
}

int BasePop::get_max_storage(int type) const {
    float storage = (get_base_need(type) + get_base_want(type)) * 3.0;
    if (storage > 0) {
        return std::max(2, int(ceil(storage)));
    }
    return 0; // Storage is based on need, so pops have ~3 + 1 months without any access before bad
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
    return std::fmin((internal_storage.at(10) / get_base_needs().at(10)), 1.0);
}

bool BasePop::will_degrade() const {
    return months_without_job > 5;
}

void BasePop::degrade() {
    if (pop_type == rural) {
        pop_type = peasant;
        reset_and_fill_storage();
        months_without_job = 0;
    }
}

bool BasePop::will_upgrade() const {
    return (pop_type == peasant && wealth > 10000);
}

void BasePop::upgrade() {
    if (pop_type == peasant) {
        pop_type = rural;
        reset_and_fill_storage();
    }
}

void BasePop::reset_and_fill_storage() {
    internal_storage.clear();
    for (const auto &[type, __]: base_needs[pop_type]) {
        internal_storage[type] = get_max_storage(type);
    }
    for (const auto &[type, __]: specialities[pop_type]) {
        internal_storage[type] = 0;
    }
}

void BasePop::month_tick() {
    for (const auto& [type, __]: internal_storage) {
        internal_storage[type] -= (get_base_need(type) + get_base_want(type));
        if (internal_storage[type] < 0) {
            // DO something
            internal_storage[type] = 0;
            if (type == 10) {
                months_starving++;
            }
        } else if (type == 10) {
            months_starving = 0;
        }
    }

    if (is_unemployed()) {
        months_without_job++;
    } else {
        months_without_job = 0;
    }
}