// my_static_registry.cpp
#include "static_registry.hpp"
#include "../singletons/recipe_info.hpp"
#include "../singletons/factory_creator.hpp"

void StaticRegistry::initialize() {
    CargoInfo::initialize_singleton();
    LocalPriceController::set_base_prices();
    RecipeInfo::create();
    FactoryCreator::create();
    
    std::unordered_map<int, float> needs;
    Ref<CargoInfo> cargo_info = CargoInfo::get_instance(); //TODO: Should be expanded and moved elsewhere
    needs[cargo_info -> get_cargo_type("grain")] = 1;
    needs[cargo_info -> get_cargo_type("wood")] = 0.3;
    needs[cargo_info -> get_cargo_type("salt")] = 0.1;
    needs[cargo_info -> get_cargo_type("fish")] = 0.2;
    needs[cargo_info -> get_cargo_type("fruit")] = 0.2;
    needs[cargo_info -> get_cargo_type("meat")] = 0.2;
    needs[cargo_info -> get_cargo_type("bread")] = 0.3;
    needs[cargo_info -> get_cargo_type("clothes")] = 0.3;
    needs[cargo_info -> get_cargo_type("furniture")] = 0.3;
    BasePop::create_base_needs(needs);
}
