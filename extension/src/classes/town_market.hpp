#pragma once

#include "hold.hpp"
#include <vector>
#include <unordered_map>

class TownMarket : public Hold {
    GDCLASS(TownMarket, Hold);
    int cash = 1000;
    std::vector<int> supply = {};
    std::vector<int> demand = {};
    std::vector<float> prices = {};
    std::vector<int> last_month_demand = {};
    std::vector<int> last_month_supply = {};

protected:
    void static _bind_methods();

public:
    TownMarket();
    void create_storage();
    std::vector<int>& get_supply();
    std::vector<int>& get_demand();
    void add_cash(float amount) override;
    void remove_cash(float amount) override;
    float get_cash() const override; 
    float transfer_cash(float amount) override;

    float get_fulfillment(int type);
    void report_attempt_to_sell(int type, int amount);
    float get_local_price(int type);
    bool is_price_acceptable(int type, float price);
    int get_desired_cargo(int type, float price);
    void buy_cargo(int type, int amount, float price);
    int sell_cargo(int type, int amount, float price);
    void adjust_prices();

    void month_tick();
};
