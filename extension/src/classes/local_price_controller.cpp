#include "local_price_controller.hpp"

#include <godot_cpp/core/class_db.hpp>

using namespace godot;

void LocalPriceController::_bind_methods() {
    ClassDB::bind_static_method(LocalPriceController::get_class_static(), D_METHOD("create", "inputs", "outputs"), &LocalPriceController::create);
    ClassDB::bind_method(D_METHOD("set_base_prices", "base"), &LocalPriceController::set_base_prices);

    ClassDB::bind_method(D_METHOD("add_cargo_type", "type", "starting_price"), &LocalPriceController::add_cargo_type, DEFVAL(-1.0f));
    ClassDB::bind_method(D_METHOD("remove_cargo_type", "type"), &LocalPriceController::remove_cargo_type);
    ClassDB::bind_method(D_METHOD("add_cargo_from_factory", "outputs"), &LocalPriceController::add_cargo_from_factory);

    ClassDB::bind_method(D_METHOD("get_change", "type"), &LocalPriceController::get_change);
    ClassDB::bind_method(D_METHOD("reset_change", "type"), &LocalPriceController::reset_change);
    ClassDB::bind_method(D_METHOD("report_change", "type", "amount"), &LocalPriceController::report_change);

    ClassDB::bind_method(D_METHOD("report_attempt", "type", "amount"), &LocalPriceController::report_attempt);
    ClassDB::bind_method(D_METHOD("reset_attempts", "type"), &LocalPriceController::reset_attempts);
    ClassDB::bind_method(D_METHOD("get_attempts", "type"), &LocalPriceController::get_attempts);

    ClassDB::bind_method(D_METHOD("get_local_price", "type"), &LocalPriceController::get_local_price);
    ClassDB::bind_method(D_METHOD("get_base_price", "type"), &LocalPriceController::get_base_price);
    ClassDB::bind_method(D_METHOD("get_percent_difference", "type"), &LocalPriceController::get_percent_difference);

    ClassDB::bind_method(D_METHOD("vary_input_price", "demand", "type"), &LocalPriceController::vary_input_price);
    ClassDB::bind_method(D_METHOD("vary_output_price", "supply", "type"), &LocalPriceController::vary_output_price);
}


LocalPriceController::LocalPriceController() {}

LocalPriceController::LocalPriceController(const std::unordered_map<int, int>& inputs, const std::unordered_map<int, int>& outputs) {
    for (const auto& [type, _] : inputs)
        add_cargo_type(type);
    for (const auto& [type, _] : outputs)
        add_cargo_type(type);
}

static LocalPriceController* create(const std::unordered_map<int, int>& inputs, const std::unordered_map<int, int>& outputs) {
    return memnew(LocalPriceController(inputs, outputs));
}

//Only one from outside c++
void set_base_prices(const Dictionary& p_base_prices) {
    Array keys = p_base_prices.keys();
    for (int i = 0; i < keys.size(); i++) {
        int type = keys[i];
        base_prices[type] = p_base_prices[type];
    }
}

void LocalPriceController::add_cargo_type(int type, float starting_price) {
    local_prices[type] = (starting_price < 0) ? base_prices[type] : starting_price;
    reset_attempts(type);
    reset_change(type);
}

void LocalPriceController::remove_cargo_type(int type) {
    local_prices.erase(type);
    attempts_to_trade.erase(type);
    change.erase(type);
}

void LocalPriceController::add_cargo_from_factory(const std::unordered_map<int, int>& outputs) {
    for (const auto& [type, _] : outputs)
        add_cargo_type(type);
}

int LocalPriceController::get_change(int type) const {
    return change.at(type);
}

void LocalPriceController::reset_change(int type) {
    change[type] = 0;
}

void LocalPriceController::report_change(int type, int amount) {
    change[type] += amount;
}

void LocalPriceController::report_attempt(int type, int amount) {
    attempts_to_trade[type] += amount;
}

void LocalPriceController::reset_attempts(int type) {
    attempts_to_trade[type] = 0;
}

int LocalPriceController::get_attempts(int type) const {
    return attempts_to_trade.at(type);
}

float LocalPriceController::get_local_price(int type) const {
    return local_prices.at(type);
}

float LocalPriceController::get_base_price(int type) const {
    return base_prices.at(type);
}

float LocalPriceController::get_percent_difference(int type) const {
    float local = local_prices.at(type);
    float base = base_prices.at(type);
    return 2.0f * (local - base) / (local + base);
}

void LocalPriceController::vary_input_price(int demand, int type) {
    int change_amt = get_change(type);
    vary_buy_order(demand, change_amt == 0 ? 0 : get_attempts(type), type);
    reset_attempts(type);
    reset_change(type);
}

void LocalPriceController::vary_output_price(int supply, int type) {
    int change_amt = get_change(type);
    vary_sell_order(change_amt == 0 ? 0 : get_attempts(type), supply, type);
    reset_attempts(type);
    reset_change(type);
}

float LocalPriceController::get_multiple(int type) const {
    return local_prices.at(type) / base_prices.at(type);
}

void LocalPriceController::vary_buy_order(int demand, int supply, int type) {
    float percentage_met = 1.0f - static_cast<float>(demand - supply) / demand;

    if (demand / 1.1f > supply)
        bump_up_good_price(type, percentage_met, 1);
    else if (demand * 1.1f < supply)
        bump_down_good_price(type, percentage_met, 2);
    else
        equalize_good_price(type);
}

void LocalPriceController::vary_sell_order(int demand, int supply, int type) {
    float percentage_met = 1.0f - static_cast<float>(supply - demand) / supply;

    if (demand / 1.1f > supply)
        bump_up_good_price(type, percentage_met, 2);
    else if (demand * 1.1f < supply)
        bump_down_good_price(type, percentage_met, 1);
    else
        equalize_good_price(type);
}

void LocalPriceController::bump_up_good_price(int type, float percentage_met, int amount) {
    float max_multiple = 0.0f;
    float price_disparity = 0.5f;
    float multiple = get_multiple(type);

    if (percentage_met != 0.0f) {
        max_multiple = 1.0f / percentage_met;
        price_disparity = std::abs(multiple - percentage_met);
    }

    if (percentage_met != 0.0f && multiple >= max_multiple)
        multiple = max_multiple;
    else if (multiple >= MAX_DIFF)
        multiple = MAX_DIFF;
    else
        multiple += std::max(price_disparity / 10.0f, 0.01f) * amount;

    local_prices[type] = base_prices[type] * multiple;
}

void LocalPriceController::bump_down_good_price(int type, float percentage_met, int amount) {
    float multiple = get_multiple(type);
    float price_disparity = std::abs(multiple - percentage_met);

    if (multiple <= percentage_met)
        multiple = percentage_met;
    else if (multiple <= 1.0f / MAX_DIFF)
        multiple = 1.0f / MAX_DIFF;
    else
        multiple -= std::max(price_disparity / 10.0f, 0.01f) * amount;

    local_prices[type] = base_prices[type] * multiple;
}

void LocalPriceController::equalize_good_price(int type) {
    float multiple = get_multiple(type);
    if (multiple > 1.0f)
        bump_down_good_price(type, 1.0f, 1);
    else if (multiple < 1.0f)
        bump_up_good_price(type, 1.0f, 1);
}

const float LocalPriceController::MAX_DIFF = 1.5f;
std::unordered_map<int, float> base_prices;