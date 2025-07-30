#include <iostream>
#include <fstream>
#include "factory_creator.hpp"
#include "../classes/town.hpp"
#include "../classes/ai_factory.hpp"
#include "../classes/construction_site.hpp"
#include "recipe_info.hpp"
#include "cargo_info.hpp"
#include "terminal_map.hpp"
#include "province_manager.hpp"
#include <godot_cpp/core/class_db.hpp>

FactoryCreator* FactoryCreator::singleton_instance = nullptr;

void FactoryCreator::_bind_methods() {
    ClassDB::bind_static_method(get_class_static(), D_METHOD("get_instance"), &FactoryCreator::get_instance);
    ClassDB::bind_method(D_METHOD("create_primary_industry", "type", "coords", "player_id", "mult"), &FactoryCreator::create_primary_industry);
    ClassDB::bind_method(D_METHOD("create_road_depot", "coords", "player_id"), &FactoryCreator::create_road_depot);
    ClassDB::bind_method(D_METHOD("create_construction_site", "coords", "player_id"), &FactoryCreator::create_construction_site);
    ClassDB::bind_method(D_METHOD("create_town", "coords"), &FactoryCreator::create_town);
}

FactoryCreator::FactoryCreator() {}
FactoryCreator::~FactoryCreator() {}

void FactoryCreator::create() {
    if (singleton_instance == nullptr) {
        singleton_instance = (memnew(FactoryCreator));
    }
}
FactoryCreator* FactoryCreator::get_instance() {
    ERR_FAIL_COND_V_MSG(singleton_instance == nullptr, nullptr, "Factory Creator not instanced");
    return singleton_instance;
}

void FactoryCreator::create_primary_industry(int type, Vector2i coords, int player_id, int mult) {
    if (coords == Vector2i(0, 0)) {
        print_error("Tried to build factory at " + coords);
        return;
    }
    Recipe* recipe = RecipeInfo::get_instance()->get_primary_recipe_for_type(type);
    
    if (player_id <= 0) {
        Ref<AiFactory> factory = Ref<AiFactory>(memnew(AiFactory(coords, player_id, recipe)));
        TerminalMap::get_instance()->encode_factory(factory, mult);
    } else {
        Ref<Factory> factory = Ref<Factory>(memnew(Factory(coords, player_id, recipe)));
        TerminalMap::get_instance()->encode_factory(factory, mult);
    }
}

void FactoryCreator::create_road_depot(Vector2i coords, int player_id) {
    Ref<ProvinceManager> province_manager = ProvinceManager::get_instance();
    Ref<RoadDepot> road_depot = Ref<RoadDepot>(memnew(RoadDepot(coords, player_id)));
    Province* province = province_manager->get_province(province_manager->get_province_id(coords));
    TerminalMap::get_instance()->create_terminal(road_depot);
    province->add_terminal(coords);
}

void FactoryCreator::create_construction_site(Vector2i coords, int player_id) {
    Ref<ProvinceManager> province_manager = ProvinceManager::get_instance();
    Ref<ConstructionSite> construction_site = Ref<ConstructionSite>(memnew(ConstructionSite(coords, player_id)));
    Province* province = province_manager->get_province(province_manager->get_province_id(coords));
    TerminalMap::get_instance()->create_terminal(construction_site);
    province->add_terminal(coords);
}

void FactoryCreator::create_town(Vector2i coords) {
    Ref<ProvinceManager> province_manager = ProvinceManager::get_instance();
    Ref<Town> town = Ref<Town>(memnew(Town(coords)));
    Province* province = province_manager->get_province(province_manager->get_province_id(coords));
    TerminalMap::get_instance()->create_terminal(town);
    province->add_terminal(coords);
}