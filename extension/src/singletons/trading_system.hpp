#pragma once

#include <vector>
#include <memory>

class Factory;
class Town;

class TradingSystem {

    static std::shared_ptr<TradingSystem> singleton;

    void adjust_factory_orders(Factory& factory, Town& town);

    void adjust_factory_orders();
    void trading_tick();

    public:
    static std::shared_ptr<TradingSystem> get_instance();
    void day_tick();
    void month_tick();
    
    
};