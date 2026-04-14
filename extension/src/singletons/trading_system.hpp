#include <vector>
#include <src/classes/map_objects/factory.hpp>


class TradingSystem {

    void adjust_factory_orders(Factory& factory, Town& town);

    public:
    void day_tick();
    
    void adjust_factory_orders();
    

    void trading_tick();
};