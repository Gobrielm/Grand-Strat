// my_static_registry.cpp
#include "static_registry.hpp"
#include "../singletons/recipe_info.hpp"
#include "../singletons/factory_creator.hpp"

void StaticRegistry::initialize() {
    CargoInfo::initialize_singleton();
    LocalPriceController::set_base_prices();
    RecipeInfo::create();
    FactoryCreator::create();
}
