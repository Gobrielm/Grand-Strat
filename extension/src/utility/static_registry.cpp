#include "static_registry.hpp"
#include "../singletons/recipe_info.hpp"
#include "../singletons/factory_creator.hpp"
#include "../singletons/user_singletons/country_manager.hpp"
#include "../singletons/pop_manager.hpp"
#include "../singletons/ai_manager.hpp"


void StaticRegistry::initialize() {

    CargoInfo::initialize_singleton();
    LocalPriceController::set_base_prices();
    RecipeInfo::create();
    FactoryCreator::create();
    
    CountryManager::create();
    PopManager::create();
    
    BasePop::create_base_needs();
    BasePop::create_base_wants();
    AiManager::create();
}

void StaticRegistry::uninitialize() {

    AiManager::cleanup();
    PopManager::cleanup();
    CountryManager::cleanup();
    FactoryCreator::cleanup();
    RecipeInfo::cleanup();
    CargoInfo::cleanup();
}

