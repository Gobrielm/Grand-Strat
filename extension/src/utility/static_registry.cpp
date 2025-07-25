// my_static_registry.cpp
#include "static_registry.hpp"
#include "../singletons/recipe_info.hpp"

void StaticRegistry::initialize() {
    CargoInfo::initialize_singleton();
    LocalPriceController::set_base_prices();
    RecipeInfo::create();
}
