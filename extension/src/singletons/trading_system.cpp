#include "trading_system.hpp"
#include "terminal_map.hpp"
#include "province_manager.hpp"

void TradingSystem::day_tick() {

}

void TradingSystem::adjust_factory_orders() {
    auto pm = ProvinceManager::get_instance();

    for (auto prov_id: pm->get_provinces_vector()) {
        auto province = pm->get_province(prov_id);

        Town& town = province->get_town();

        for (auto& [id, factory]: province->get_factories()) {
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
        auto& town = province->get_town();
        town.mp.market_tick();
    }
}