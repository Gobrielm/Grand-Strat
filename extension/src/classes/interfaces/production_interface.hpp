#pragma once
#include "classes/local_price_controller.hpp"
#include <unordered_map>
#include <memory>

class ProductionInterface {

    private:
    static constexpr int INITIAL_MAX_STORAGE = 5000;
    static constexpr float MAX_TRADE_MARGIN = 1.05f;

    float MAX_STORAGE;
    std::unordered_map<int, float> storage;
    LocalPriceController local_pricer;

    public:
    
    ProductionInterface();
    ~ProductionInterface();
    ProductionInterface(const ProductionInterface&) = delete;

    Dictionary get_local_prices() const;
    float get_local_price(int type) const;

    int get_desired_cargo(int type, float pricePer) const;

    bool is_price_acceptable(int type, float pricePer) const;


    /// @return The amount of cargo not added to local storage
    float add_cargo(int type, float amount);
    void remove_cargo(int type, float amount);

    float get_price_average(int type, const ProductionInterface& other) const;

    void add_demand(int type, float price, float amount);
    void add_demand(int type, const std::unordered_map<int, float>& ten_price_map);

    void add_supply(int type, float price, float amount);
    void add_supply(int type, const std::unordered_map<int, float>& ten_price_map);

    std::unordered_map<int, float> get_last_month_demand(int type) const;
    std::unordered_map<int, float> get_last_month_supply(int type) const;
    float get_diff_between_demand_and_supply(int type) const;
    float get_demand_at_price(int type, float price) const;

    
    Dictionary get_last_month_supply() const;
    Dictionary get_last_month_demand() const;
};
