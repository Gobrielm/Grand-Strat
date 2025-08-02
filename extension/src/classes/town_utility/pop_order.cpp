#include "pop_order.hpp"
#include <unordered_map>

PopOrder::PopOrder(const PopOrder* other) {
    type = other->type;
    amount = other->amount;
    price = other->price;
    pop_id = other->pop_id;
}

PopOrder::PopOrder(int p_type, int p_amount, float p_price, int p_pop_id) {
    type = p_type;
    amount = p_amount;
    price = p_price;
    pop_id = p_pop_id;
}

void PopOrder::buy_cargo(int p_amount) {
    amount -= p_amount;
}

// Sorts by price
bool PopOrder::operator>(const PopOrder& other) const {
    if (price == other.price) {
        return this < &other;
    }
    return price < other.price; // Highest price first
}
bool PopOrder::operator==(const PopOrder& other) const {
    return this == &other;
}