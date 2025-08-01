#include "town_cargo.hpp"
#include "../factory_template.hpp"
#include "../../singletons/terminal_map.hpp"

TownCargo::TownCargo(const TownCargo* other) {
    type = other->type;
    amount = other->amount;
    price = other->price;
    terminal_id = other->terminal_id;
    fees_to_pay = other->fees_to_pay;
    age = 0;
}

TownCargo::TownCargo(int p_type, int p_amount, float p_price, int p_terminal_id) {
    type = p_type;
    amount = p_amount;
    price = p_price;
    terminal_id = p_terminal_id;
}

bool TownCargo::operator>(const TownCargo& other) const {
    if (price == other.price)
        return this > &other; // fallback to pointer address
    return price > other.price;
}

bool TownCargo::operator==(const TownCargo& other) const {
    return this == &other; // identity comparison
}

void TownCargo::sell_cargo(int p_amount) {
    sell_cargo(p_amount, price);
}

void TownCargo::sell_cargo(int p_amount, float p_price) {
    Ref<FactoryTemplate> factory = TerminalMap::get_instance()->get_terminal_as<FactoryTemplate>(terminal_id);
    if (factory.is_null()) return; 
    float total_price = p_amount * p_price;
    pay_fees(total_price);
    factory->add_cash(total_price);
    amount -= p_amount;
}

void TownCargo::sell_cargo(int p_amount, float p_price, std::unordered_map<int, float>& to_pay) {
    float total_price = p_amount * p_price;
    for (auto& [term_id, fee]: fees_to_pay) {
        if (!to_pay.count(term_id)) {
            to_pay[term_id] = 0;
        }
        float fee_price = total_price * fee;
        to_pay[term_id] += fee_price;
        total_price -= fee_price;
    }
    if (!to_pay.count(terminal_id)) {
        to_pay[terminal_id] = 0;
    }
    to_pay[terminal_id] += total_price;
    amount -= p_amount;
}

void TownCargo::pay_fees(float &total_price) {
    for (auto& [terminal_id, fee]: fees_to_pay) {
        float to_pay = total_price * fee;
        TerminalMap::get_instance()->get_terminal_as<RoadDepot>(terminal_id)->add_cash(to_pay);
        total_price -= to_pay;
    }
}

void TownCargo::transfer_cargo(int p_amount) {
    amount -= p_amount;
}

void TownCargo::return_cargo() {
    Ref<FactoryTemplate> factory = TerminalMap::get_instance()->get_terminal_as<FactoryTemplate>(terminal_id);
    if (factory.is_null()) return; 
    factory->add_cargo(type, amount);
    amount = 0;
}

void TownCargo::add_fee_to_pay(int term_id, float fee) {
    if (!fees_to_pay.count(term_id)) {
        fees_to_pay[term_id] = 0;
    }
    fees_to_pay[term_id] += fee;
}