#pragma once

#include "broker.hpp"
#include <vector>
#include <unordered_map>

class TownMarket : public Broker {
    GDCLASS(TownMarket, Broker);
    int cash = 1000;

protected:
    void static _bind_methods();

public:
    TownMarket();
    void create_storage();
    const std::vector<int>& get_supply();
    const std::vector<int>& get_demand();

    void add_cash(float amount) override;
    void remove_cash(float amount) override;
    float get_cash() const override; 
    float transfer_cash(float amount) override;

    float get_fulfillment(int type);
    bool is_price_acceptable(int type, float price) const override;
    int get_desired_cargo(int type, float price) const override;
    

    void month_tick();
};
