#include "capital_interface.hpp"

#include "../singletons/money_controller.hpp"

CapitalInterface::CapitalInterface(): cash(INITIAL_CASH) {}

int CapitalInterface::get_amount_can_buy(const float amount_per) const {
    return amount_per <= 0 ? 10000000 : floor(get_cash() / amount_per);
}

void CapitalInterface::add_cash(float amount) {
    cash += amount;
    cash_history.back() += amount;
}

void CapitalInterface::remove_cash(float amount) {
    cash -= amount;
    cash_history.back() -= amount;
}

float CapitalInterface::get_cash() const {
    return cash;
}

float CapitalInterface::transfer_cash(float amount) {
    amount = std::min(get_cash(), amount);
	remove_cash(amount);
	return amount;
}
    
void CapitalInterface::update_cash_history() {
    cash_history.pop_front();
}

const std::list<float>& CapitalInterface::get_cash_history() const {
    return cash_history;
}
