#pragma once

#include <vector>
#include <memory>

class SubsistenceFarm;
class Factory;
class Town;

class TradingSystem {

    static std::shared_ptr<TradingSystem> singleton;

    public:
    static std::shared_ptr<TradingSystem> get_instance();
    void order_tick();
    void trading_tick();
    
    
};