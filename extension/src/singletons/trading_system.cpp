#include "trading_system.hpp"
#include "terminal_map.hpp"
#include "province_manager.hpp"

void TradingSystem::day_tick() {

}

void TradingSystem::adjust_factory_orders() {
    auto pm = ProvinceManager::get_instance();

    for (auto prov_id: pm->get_provinces_vector()) {
        auto province = pm->get_province(prov_id);

        std::scoped_lock lock(province->m);
        
        Town& town = province->town;

        for (auto& factory: province->factories) {
            adjust_factory_orders(factory, town);
        }
    }
}

void TradingSystem::adjust_factory_orders(Factory& factory, Town& town) {
    for (const auto& [type, _]: factory.recipe.get_outputs()) {
        factory.adjust_trade_orders(town);
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