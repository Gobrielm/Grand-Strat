#pragma once

#include <godot_cpp/classes/ref_counted.hpp>
#include <unordered_map>
#include <map>

using namespace godot;

class LocalPriceController {
    protected:

    static std::vector<float> base_prices;
    
    // All of these are surveyed from other markets and represent a host of buyers' and sellers' prices

    std::unordered_map<int, std::unordered_map<int, float>> demand; // type -> price * 10 -> amount
    std::unordered_map<int, std::unordered_map<int, float>> supply; // type -> price * 10 -> amount
    std::unordered_map<int, std::unordered_map<int, float>> last_month_demand; // type -> price * 10 -> amount
    std::unordered_map<int, std::unordered_map<int, float>> last_month_supply; // type -> price * 10 -> amount

    std::unordered_map<int, float> current_prices; // Keep track of prices from last month

    
    float get_difference_from_base_price(int type, std::vector<float> &p_supply, std::vector<float> &p_demand) const;


    virtual void update_local_price(int type);

    double get_weighted_average(std::unordered_map<int, float> &m) const;

    public:

    LocalPriceController();

    static std::vector<float> get_base_prices();
    static void set_base_prices();

    virtual void update_local_prices();

    void add_demand(int type, float price, float amount); //Demand is only from attempts to buy/sell
    void add_supply(int type, float price, float amount); //Supply is only from bought/created goods
    float get_demand(int type) const;
    float get_supply(int type) const;
    float get_last_month_demand(int type) const;
    float get_last_month_supply(int type) const;


    std::unordered_map<int, float> get_demand() const;
    std::unordered_map<int, float> get_supply() const;
    std::unordered_map<int, float> get_last_month_demand() const;
    std::unordered_map<int, float> get_last_month_supply() const;

    std::unordered_map<int, float> get_last_month_demand_ten_price_map(int type) const;
    std::unordered_map<int, float> get_last_month_supply_ten_price_map(int type) const;

    Dictionary get_last_month_demand_dict() const;
    Dictionary get_last_month_supply_dict() const;

    virtual float get_demand_at_price(int type, float price) const;
    virtual float get_supply_at_price(int type, float price) const;

    float get_local_price(int type) const;
    static float get_base_price(int type);

    std::unordered_map<int, float> get_local_prices() const;
    Dictionary get_local_prices_dict();

};
