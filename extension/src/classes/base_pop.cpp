#include "base_pop.hpp"
#include "classes/trade_order.hpp"
#include "classes/map_objects/town.hpp"
#include "singletons/terminal_map.hpp"
#include "singletons/cargo_info.hpp"
#include "singletons/data_collector.hpp"
#include <godot_cpp/core/class_db.hpp>
#include <sstream>
#include <fstream>
#include <iostream>

std::atomic<int> BasePop::total_pops = 0;

std::unordered_map<PopTypes, int> BasePop::PEOPLE_PER_POP = {
    {PopTypes::rural, 1000},
    {PopTypes::town, 1000},
    {PopTypes::peasant, 1000},
    {PopTypes::none, 0}
};

std::unordered_map<PopTypes, int> BasePop::INITIAL_WEALTH = {
    {PopTypes::rural, 1000},
    {PopTypes::town, 1000},
    {PopTypes::peasant, 1000},
    {PopTypes::none, 0}
};

std::unordered_map<PopTypes, std::unordered_map<int, float>> BasePop::base_needs;

std::unordered_map<PopTypes, std::unordered_map<int, float>> BasePop::specialities;

BasePop* BasePop::create_rural_pop(int p_home_prov_id, Vector2i p_location, Variant p_culture) {
    return memnew(BasePop(p_home_prov_id, p_location, p_culture, PopTypes::rural));
}
BasePop* BasePop::create_peasant_pop(int p_home_prov_id, Vector2i p_location, Variant p_culture) {
    return memnew(BasePop(p_home_prov_id, p_location, p_culture, PopTypes::peasant));
}
BasePop* BasePop::create_town_pop(int p_home_prov_id, Vector2i p_location, Variant p_culture) {
    return memnew(BasePop(p_home_prov_id, p_location, p_culture, PopTypes::town));
}

BasePop::BasePop(): BasePop(-1, Vector2i(0, 0), -1, PopTypes::none) {}

BasePop::BasePop(int p_home_prov_id, Vector2i p_location, Variant p_culture, PopTypes p_pop_type): pop_id(total_pops++) {
    location = p_location;
    home_prov_id = p_home_prov_id;
    culture = p_culture;
    pop_type = p_pop_type;
    
    capital.set_cash(INITIAL_WEALTH.at(pop_type));
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
        {"peasant", PopTypes::peasant},
        {"rural", PopTypes::rural},
        {"town", PopTypes::town},
        {"none", PopTypes::none}
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

void BasePop::set_type(PopTypes p_pop_type) {
    pop_type = p_pop_type;
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
    
    capital.add_cash(wage);
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

float BasePop::get_expected_income(std::unordered_map<int, float> current_prices) const {
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

void BasePop::add_wealth(double amount) {
    capital.add_cash(amount);
    income += amount;
}

void BasePop::add_wealth_no_change_to_income(double amount) {
    capital.add_cash(amount);
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
    float available_money = std::min(std::max(income, current_price * mult), capital.get_cash()); // Highest will go

    return std::min(available_money, current_price);
}

float BasePop::get_buy_price_for_wanted_good(int type, float current_price) const {
    if (!are_needs_met() || income == 0) {
        return 0;
    }

    float available_money = income;
    
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
    for (const auto &[type, amount_needed]: base_needs.at(pop_type)) {
        if (storage.get_amount(type) < amount_needed) {
            return false;
        }
    }
    return true;
}

bool BasePop::is_need_met(int type) const {
    return (storage.get_amount(type) >= get_base_need(type));
}

bool BasePop::is_want_met(int type) const {
    return (storage.get_amount(type) >= (get_base_need(type) + get_base_want(type)));
}

unsigned int BasePop::get_desired(int type) const {
    if (!base_needs.at(pop_type).count(type) && !specialities.at(pop_type).count(type)) {
        return 0;
    }
    int amount = std::max(int(get_max_storage(type) - storage.get_amount(type)), 0);
    if (income == 0 && !get_base_needs().count(type)) { // Don't buy if not neccessary and no job
        return 0; 
    }
	
	return amount;
}

unsigned int BasePop::get_desired(int type, float price) const {
    ERR_FAIL_COND_V_MSG(price < 0, 0, "Price is below 0.");
    int amount_can_buy = int(capital.get_cash() / price);
    int amount_can_store = int(get_max_storage(type) - storage.get_amount(type));
    int amount = std::min(amount_can_buy, amount_can_store);

    if (income == 0.0 && !get_base_needs().count(type)) {
        return 0; // Don't buy if not neccessary and no job
    }
	
	return std::max(amount, 0);
}

void BasePop::buy_good(int type, int amount, float price) {
    if (std::isnan(price)) {
        ERR_FAIL_MSG("NaN in buy_good inputs");
    }
    if (amount < 0) {
        ERR_FAIL_MSG("Amount is negitive");
    }
    capital.remove_cash(amount * price);
    storage.add_cargo(type, amount);

	if (capital.get_cash() < 0) { //TODO, uh-oh
        ERR_FAIL_MSG("Not enough money to buy good as pop");
        capital.set_cash(0);
    }
}

void BasePop::add_cargo(int type, int amount) {
    storage.add_cargo(type, amount);
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
    return capital.get_cash();
}

float BasePop::transfer_wealth() {
    float toReturn = get_wealth() * 0.5;
	capital.remove_cash(toReturn);
	return toReturn;
}

Variant BasePop::get_culture() const {
    return culture;
}

float BasePop::get_fulfillment(int type) const {
    int tot = get_base_need(type) + get_base_want(type);
    if (tot == 0) return 1;

    return std::fmin(storage.get_amount(type) / tot, 1.0);
}

float BasePop::get_average_fulfillment() const {
    return std::fmin((storage.get_amount(10) / get_base_needs().at(10)), 1.0);
}

bool BasePop::will_degrade() const {
    return months_without_job > 5;
}

void BasePop::degrade() {
    if (pop_type == PopTypes::rural) {
        pop_type = PopTypes::peasant;
        reset_and_fill_storage();
        months_without_job = 0;
    }
}

bool BasePop::will_upgrade() const {
    return (pop_type == PopTypes::peasant);
}

void BasePop::upgrade() {
    if (pop_type == PopTypes::peasant) {
        pop_type = PopTypes::rural;
        reset_and_fill_storage();
    }
}

void BasePop::reset_and_fill_storage() {
    storage = StorageComponent();
    for (const auto &[type, __]: base_needs[pop_type]) {
        storage.set_cargo(type, get_max_storage(type));
    }
    for (const auto &[type, __]: specialities[pop_type]) {
        storage.set_cargo(type, 0);
    }
}

void BasePop::adjust_pop_orders(Town& town) { // Remove pop orders, super dumb

    auto adjust_pop_want_orders = [&] () {
        for (const auto& [type, amt]: get_base_wants(pop_type)) {

            double max_price = get_buy_price_for_wanted_good(type, town.mp.get_price(type));

            if (!orders.count(type)) {
                orders[type] = std::make_shared<TradeOrder>(pop_id, TradeOrderOwner::POP, type, amt, true, town.mp.get_price(type), max_price);
                town.mp.add_order(orders[type]);
            }

            orders[type]->change_amount(get_desired(type));

            float price = orders[type]->get_price();
            orders[type]->set_max_price(max_price);

            if (get_fulfillment(type) < 0.75) {
                float new_price = std::min(max_price, price * 1.01);
                orders[type]->set_price(new_price);
            } else {
                orders[type]->set_price(price / 1.01);
            }
        }
    };

    auto adjust_pop_need_orders = [&] () {
        for (const auto& [type, amt]: get_base_needs(pop_type)) {
            if (!orders.count(type)) {
                orders[type] = std::make_shared<TradeOrder>(pop_id, TradeOrderOwner::POP, type, amt, true, town.mp.get_price(type), get_buy_price_for_needed_good(type, town.mp.get_price(type)));
                town.mp.add_order(orders[type]);
            }

            orders[type]->change_amount(get_desired(type));

            float price = orders[type]->get_limit_price();
            orders[type]->set_max_price(get_buy_price_for_needed_good(type, town.mp.get_price(type)));

            if (get_fulfillment(type) < 0.75) {
                float new_price = std::min(orders[type]->get_limit_price(), price * 1.01);
                orders[type]->set_price(new_price);
            } else {
                orders[type]->set_price(price / 1.01);
            }
        }
    };

    adjust_pop_want_orders();
    adjust_pop_need_orders();
    
    int grain_type = CargoInfo::get_instance()->get_cargo_type("grain");
    if (orders.count(grain_type)) {
        DataCollector::get_instance()->add_demand(grain_type, orders[grain_type]->get_amount());
    }
}

void BasePop::month_tick() {
    for (const auto& [type, amt]: storage.get_storage()) {
        float to_use = get_base_need(type) + get_base_want(type);
        storage.remove_cargo(type, std::min(amt, to_use));
        
        if (amt < to_use) {
            // DO something
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