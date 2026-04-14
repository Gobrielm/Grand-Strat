#pragma once

#include "classes/local_price_controller.hpp"
#include <classes/trade_order.hpp>

#include <set>
#include <unordered_map>
#include <memory>
#include <optional>

class MarketComponent {

    private:

    std::unordered_map<int, 
        std::set<std::shared_ptr<TradeOrder>, 
            std::vector<std::shared_ptr<TradeOrder>>,
            TradeOrder::TradeOrderLT()
        >
    > sell_orders;

    std::unordered_map<int, 
        std::set<std::shared_ptr<TradeOrder>, 
            std::vector<std::shared_ptr<TradeOrder>>,
            TradeOrder::TradeOrderGT()
        >
    > buy_orders;

    std::optional<std::pair<CapitalComponent&, StorageComponent&>> get_capital_and_storage_components(int pos_id);
    void finish_market_exchange(std::shared_ptr<TradeOrder> buy_order, std::shared_ptr<TradeOrder> sell_order, std::pair<CapitalComponent&, StorageComponent&> buyer, std::pair<CapitalComponent&, StorageComponent&> seller);

    public:
    
    MarketComponent();
    MarketComponent(const MarketComponent& other);

    void add_order(std::shared_ptr<TradeOrder> to);
    float get_price(int type) const;

    std::pair<std::vector<std::pair<int, float>>, std::vector<std::pair<int, float>>> get_market_info_plot(int type) const;
    void market_tick();

    long get_current_demand(int type) const;
    long get_current_supply(int type) const;

    void sell_to_pop(BasePop& pop);

};
