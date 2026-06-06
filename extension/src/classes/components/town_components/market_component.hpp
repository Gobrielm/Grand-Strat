#pragma once

#include "classes/local_price_controller.hpp"
#include "classes/trade_order.hpp"

#include <deque>
#include <set>
#include <unordered_map>
#include <memory>
#include <optional>

class Province;
class CapitalComponent;
class StorageComponent;
class BasePop;

class MarketComponent {

    private:

    std::vector<std::shared_ptr<TradeOrder>> buy_orders;
    std::vector<std::shared_ptr<TradeOrder>> sell_orders;

    // Descending Orders
    std::unordered_map<int, 
        std::deque<TradeOrder>
    > sorted_buy_orders;

    // Ascending Orders
    std::unordered_map<int, 
        std::deque<TradeOrder>
    > sorted_sell_orders;

    std::unordered_map<int, float> equilibrium_prices;

    static const int PLOT_BUCKETS = 200;
    // index by type, then by price, buckets of 0.5, ie round(price * 2) to get amount
    std::vector<unsigned int> sale_history;
    // index by type, then by price, buckets of 0.5, ie round(price * 2) to get amount
    std::vector<unsigned int> buy_order_buckets;
    // index by type, then by price, buckets of 0.5, ie round(price * 2) to get amount
    std::vector<unsigned int> sell_order_buckets;

    std::vector<int> get_types_among_orders() const;

    void finish_market_exchange(TradeOrder& buy_order, TradeOrder& sell_order, std::pair<CapitalComponent*, StorageComponent*> buyer, std::pair<CapitalComponent*, StorageComponent*> seller);

    std::pair<std::vector<std::pair<int, float>>, std::vector<std::pair<int, float>>> get_market_info_plot(int type) const;
    void sort_orders();

    public:
    
    MarketComponent();
    MarketComponent(const MarketComponent& other);


    void add_order(std::shared_ptr<TradeOrder> to);
    float get_price(int type) const;
    float get_min_price(int type) const;
    float get_max_price(int type) const;

    std::unordered_map<int, float> get_current_prices() const;
    Array get_market_info_plot_godot(int type) const;

    Array get_market_sale_history(int type) const;
    std::vector<unsigned int> get_market_sale_history_ref(int type) const;
    
    /// @param province The locked province that town is in
    void market_tick(Province* province);

    /// @param province The locked province that town is in
    void sell_to_pop(Province* province, BasePop& pop);

    int32_t get_current_demand(int type) const;
    int32_t get_current_supply(int type) const;

    void bookkeeping_tick();
    float find_market_price_equilibrium(int type) const;

    void record_sale(int type, double price, int amount);
};