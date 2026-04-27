#include "map_object_info.hpp"
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/classes/ref.hpp>

#include "singletons/province_manager.hpp"
#include "classes/province.hpp"
#include "classes/map_objects/town.hpp"
#include "classes/map_objects/factory.hpp"

void MapObjectInfo::_bind_methods() {
    ClassDB::bind_static_method(get_class_static(), D_METHOD("get_town_factories", "town_tile"), &MapObjectInfo::get_town_factories);
    ClassDB::bind_static_method(get_class_static(), D_METHOD("get_town_prices", "town_tile"), &MapObjectInfo::get_town_prices);
}

MapObjectInfo::MapObjectInfo() {

}

Array MapObjectInfo::get_town_factories(Vector2i town_tile) {
    auto pm = ProvinceManager::get_instance();
    auto province = pm->get_province(town_tile);

    std::scoped_lock lock(province->m);

    auto ids = province->town.get_factory_ids();

    Array a;
    for (auto id: ids) {
        Factory& factory = province->get_factory(id);
        Dictionary d;
        d["level"] = factory.employer.get_level_without_employment();
        d["cash"] = factory.capital.get_cash();
        d["recipe"] = factory.get_recipe().get_recipe_as_string();
        a.push_back(d);
    }

    return a;
}

Dictionary MapObjectInfo::get_town_prices(Vector2i town_tile) {
    auto pm = ProvinceManager::get_instance();
    auto province = pm->get_province(town_tile);

    std::scoped_lock lock(province->m);

    Town& town = province->town;
    Dictionary d;
    auto price_map = town.mp.get_current_prices();

    for (auto [type, price]: price_map) {
        Dictionary inner_info;

        inner_info["price"] = price;
        inner_info["supply"] = town.mp.get_current_supply(type);
        inner_info["demand"] = town.mp.get_current_demand(type);
        inner_info["plot"] = town.mp.get_market_info_plot_godot(type);

        d[type] = inner_info;
    }

    return d;
}