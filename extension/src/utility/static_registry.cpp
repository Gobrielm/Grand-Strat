#include "static_registry.hpp"
#include "../singletons/recipe_info.hpp"
#include "../singletons/user_singletons/country_manager.hpp"
#include "../singletons/pop_manager.hpp"
#include "debug_trace.h"


void StaticRegistry::initialize() {

    CargoInfo::initialize_singleton();
    LocalPriceController::set_base_prices();
    RecipeInfo::create();
    // FactoryCreator::create();
    
    CountryManager::create();
    PopManager::create();
    
    BasePop::create_base_needs();
    BasePop::create_base_wants();

    DebugTrace::create_instance();
    // AiManager::create();
}

void StaticRegistry::uninitialize() {

    // AiManager::cleanup();
    PopManager::cleanup();
    CountryManager::cleanup();
    // FactoryCreator::cleanup();
    RecipeInfo::cleanup();
    CargoInfo::cleanup();
}

