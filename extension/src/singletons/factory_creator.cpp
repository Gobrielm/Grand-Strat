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
    return singleton_instance;
}

void FactoryCreator::create_primary_industry(int type, Vector2i coords, int player_id, int mult) {
    Recipe* recipe = RecipeInfo::get_instance()->get_primary_recipe_for_type(type);
    Ref<Factory> factory;
    if (player_id <= 0) {
        factory = Ref<AiFactory>(memnew(AiFactory())); // This may not create an ai factory
    } else {
        factory.instantiate();
    }
    factory->initialize(coords, player_id, recipe);
    TerminalMap::get_instance()->encode_factory(factory, mult);
}