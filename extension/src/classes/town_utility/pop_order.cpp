#include "pop_order.hpp"

PopOrder::PopOrder(const PopOrder* other) {
    type = other->type;
    amount = other->amount;
    exp_price = other->exp_price;
    limit_price = other->limit_price;
    pop_id = other->pop_id;
}

PopOrder::PopOrder(int p_type, int p_amount, float p_exp_price, float p_limit_price, int p_pop_id) {
    type = p_type;
    amount = p_amount;
    exp_price = p_exp_price;
    limit_price = p_limit_price;
    pop_id = p_pop_id;
}

void PopOrder::buy_cargo(int p_amount) {
    amount -= p_amount;
}

// Sorts by price
bool PopOrder::operator>(const PopOrder& other) const {
    if (limit_price == other.limit_price) {
        return this < &other;
    }
    return limit_price < other.limit_price; // Highest price first
}
bool PopOrder::operator==(const PopOrder& other) const {
    return this == &other;
}