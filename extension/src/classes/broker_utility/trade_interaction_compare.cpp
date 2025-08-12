#include "trade_interaction_compare.hpp"
#include "trade_interaction.hpp"

bool TradeInteractionPtrCompare::operator()(const TradeInteraction* lhs, const TradeInteraction* rhs) const {
    if (lhs->price == rhs->price)
        return lhs < rhs; // fallback to pointer address
    return lhs->price > rhs->price; // bigger price first
}