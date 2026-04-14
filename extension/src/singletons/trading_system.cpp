#include "trading_system.hpp"
#include "terminal_map.hpp"
#include "province_manager.hpp"

void TradingSystem::day_tick() {

}

void TradingSystem::adjust_factory_orders(std::vector<Factory>& factories) {
    auto pm = ProvinceManager::get_instance();
    auto tm = TerminalMap::get_instance();

    for (auto prov_id: pm->get_provinces_vector()) {
        auto province = pm->get_province(prov_id);
        std::vector<Vector2i> town_tiles = province->get_town_tiles();
        if (town_tiles.empty()) continue;

        int town_id = tm->cargo_map_terminals[town_tiles[0]];
        Town& town = tm->towns[town_id];

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
    auto tm = TerminalMap::get_instance();

    for (auto& [_, town]: tm->get_towns()) {
        town.mp.market_tick();
    }
}