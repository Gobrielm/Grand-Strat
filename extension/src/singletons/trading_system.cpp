#include "trading_system.hpp"
#include "terminal_map.hpp"
#include "province_manager.hpp"

#include "classes/map_objects/subsistence_farm.hpp"
#include "classes/map_objects/factory.hpp"
#include "classes/map_objects/town.hpp"

std::shared_ptr<TradingSystem> TradingSystem::singleton = std::make_shared<TradingSystem>();

std::shared_ptr<TradingSystem> TradingSystem::get_instance() {
    return singleton;
}

void TradingSystem::day_tick() {

}

void TradingSystem::month_tick() {
    adjust_factory_orders();
    trading_tick();
}

void TradingSystem::adjust_factory_orders() {
    auto pm = ProvinceManager::get_instance();

    for (auto prov_id: pm->get_provinces_vector()) {
        auto province = pm->get_province(prov_id);

        std::scoped_lock lock(province->m);
        
        Town& town = province->town;

        for (auto& factory: province->factories) {
            factory.adjust_trade_orders(town);
        }
        for (auto& farm: province->sub_farms) {
            farm.adjust_trade_orders(town);
        }
    }
}

void TradingSystem::trading_tick() {
    auto pm = ProvinceManager::get_instance();

    for (auto prov_id: pm->get_provinces_vector()) {
        auto province = pm->get_province(prov_id);
        
        std::scoped_lock lock(province->m);
        auto& town = province->town;
        town.mp.market_tick(province);
    }
}