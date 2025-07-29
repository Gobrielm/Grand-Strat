#include <iostream>
#include <fstream>
#include "factory_creator.hpp"
#include "../classes/Specific_Buildings/wheat_farm.hpp"
#include "recipe_info.hpp"
#include "cargo_info.hpp"
#include "terminal_map.hpp"
#include <godot_cpp/core/class_db.hpp>

FactoryCreator* FactoryCreator::singleton_instance = nullptr;

void FactoryCreator::_bind_methods() {
    ClassDB::bind_static_method(get_class_static(), D_METHOD("get_instance"), &FactoryCreator::get_instance);
    ClassDB::bind_method(D_METHOD("create_primary_industry", "type", "coords", "player_id", "mult"), &FactoryCreator::create_primary_industry);

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