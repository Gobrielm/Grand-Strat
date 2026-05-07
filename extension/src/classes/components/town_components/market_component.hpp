#pragma once

#include "classes/local_price_controller.hpp"
#include "classes/trade_order.hpp"

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

    std::unordered_map<int, 
        std::set<std::shared_ptr<TradeOrder>, TradeOrder::TradeOrderLT>
    > sell_orders;

    std::unordered_map<int, 
        std::set<std::shared_ptr<TradeOrder>, TradeOrder::TradeOrderGT>
    > buy_orders;

    // <amount, price>
    std::unordered_map<int, std::pair<std::vector<std::pair<int, float>>, std::vector<std::pair<int, float>>>> last_month_plot;
    std::unordered_map<int, std::vector<std::pair<int, float>>> population_demand;

    std::pair<CapitalComponent*, StorageComponent*> get_capital_and_storage_components(Province* province, int pos_id);
    void finish_market_exchange(std::shared_ptr<TradeOrder> buy_order, std::shared_ptr<TradeOrder> sell_order, std::pair<CapitalComponent*, StorageComponent*> buyer, std::pair<CapitalComponent*, StorageComponent*> seller);

    public:
    
    MarketComponent();
    MarketComponent(const MarketComponent& other);

    void add_order(std::shared_ptr<TradeOrder> to);
    float get_price(int type) const;
    float get_min_price(int type) const;
    float get_max_price(int type) const;

    std::unordered_map<int, float> get_current_prices() const;

    std::pair<std::vector<std::pair<int, float>>, std::vector<std::pair<int, float>>> get_market_info_plot(int type) const;
    Array get_market_info_plot_godot(int type) const;
    
    /// @param province The locked province that town is in
    void market_tick(Province* province);

    int32_t get_current_demand(int type) const;
    int32_t get_current_supply(int type) const;

    /// @param province The locked province that town is in
    void sell_to_pop(Province* province, BasePop& pop);

    void update_last_month_plot();
};