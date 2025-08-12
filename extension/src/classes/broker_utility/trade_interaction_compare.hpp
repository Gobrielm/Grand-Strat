#pragma once

struct TradeInteraction;

struct TradeInteractionPtrCompare {
    bool operator()(const TradeInteraction* lhs, const TradeInteraction* rhs) const;
};

