#pragma once

#include "hold.hpp"
#include <vector>
#include <unordered_set>

class TownMarket : public Hold {
    int cash;
    std::vector<int> supply;
    std::vector<int> demand;
    std::vector<float> prices;

public:
    TownMarket();
    std::vector<int>& get_supply();
    void add_cash(float amount);
    void remove_cash(float amount);
    float get_cash();
    float transfer_cash(float amount);

    float get_fulfillment(int type);
    void report_attempt_to_sell(int type, int amount);
    float get_local_price(int type);
    bool is_price_acceptable(int type, float price);
    void buy_cargo(int type, int amount, float price);
    int sell_cargo(int type, int amount, float price);
    void adjust_prices();

    void month_tick();
};
