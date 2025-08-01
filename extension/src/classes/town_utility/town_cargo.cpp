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
    return price > other.price;
}

bool TownCargo::operator==(const TownCargo& other) const {
    return this == &other; // identity comparison
}

void TownCargo::sell_cargo(int p_amount) {
    Ref<Broker> broker = TerminalMap::get_instance()->get_terminal_as<Broker>(terminal_id);
    if (broker.is_null()) return; 
    float total_price = p_amount * price;
    pay_fees(total_price);
    broker->add_cash(total_price);
    transfer_cargo(p_amount);
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