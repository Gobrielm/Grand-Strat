#pragma once

#include <godot_cpp/classes/ref_counted.hpp>
#include <unordered_map>

using namespace godot;

class LocalPriceController: public RefCounted {
    GDCLASS(LocalPriceController, RefCounted);
    
    static std::vector<float> base_prices;
    static constexpr float MARKET_CHANGE_RATE = 0.1f; //Higher the faster
    static constexpr float TRADE_CHANGE_RATE = 0.05f; //Price changing from trades

    std::vector<int> demand;
    std::vector<int> last_month_demand;
    std::vector<int> supply;
    std::vector<int> last_month_supply;
    std::vector<float> local_prices;

    
    void adjust_cargo_price(int type, float base_price);
    float get_difference_from_base_price(int type, std::vector<int> &p_supply, std::vector<int> &p_demand) const;

    protected:
    static void _bind_methods();

    public:
    static constexpr float MAX_DIFF = 1.5f; //Highest difference from base price for all traders

    LocalPriceController();

    static std::vector<float> get_base_prices();
    static void set_base_prices();

    void move_price(int type, float price);
    void adjust_prices();
    float get_current_difference_from_base_price(int type);

    void add_demand(int type, int amount); //Demand is only from attempts to buy/sell
    void add_supply(int type, int amount); //Supply is only from bought/created goods
    int get_demand(int type) const;
    int get_supply(int type) const;
    int get_last_month_demand(int type) const;
    int get_last_month_supply(int type) const;


    std::vector<int> get_demand() const;
    std::vector<int> get_supply() const;
    std::vector<int> get_last_month_demand() const;
    std::vector<int> get_last_month_supply() const;


    float get_local_price(int type) const;
    static float get_base_price(int type);

    Dictionary get_local_prices();

    virtual float get_max_diff() const;
};
