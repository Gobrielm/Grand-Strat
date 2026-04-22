#include "capital_component.hpp"

CapitalComponent::CapitalComponent(): cash(INITIAL_CASH) {
    for (int i = 0; i < 30; i++) {
        cash_history.push_back(0);
    }
}

int CapitalComponent::get_amount_can_buy(const float amount_per) const {
    return amount_per <= 0 ? 10000000 : floor(get_cash() / amount_per);
}

void CapitalComponent::add_cash(float amount) {
    cash += amount;
    cash_history.back() += amount;
}

void CapitalComponent::remove_cash(float amount) {
    cash -= amount;
    cash_history.back() -= amount;
}

float CapitalComponent::get_cash() const {
    return cash;
}

float CapitalComponent::transfer_cash(float amount) {
    amount = std::min(get_cash(), amount);
	remove_cash(amount);
	return amount;
}
    
void CapitalComponent::update_cash_history() {
    cash_history.pop_front();
}

const std::list<float>& CapitalComponent::get_cash_history() const {
    return cash_history;
}

const float CapitalComponent::get_recent_change() const {
    return cash_history.back() - *std::next(cash_history.end(), -2);
}