#include "subsistence_farm.hpp"
#include "../../singletons/recipe_info.hpp"

void SubsistenceFarm::_bind_methods() {
    ClassDB::bind_method(D_METHOD("month_tick"), &SubsistenceFarm::month_tick);
    
}

SubsistenceFarm::SubsistenceFarm(): IsolatedBroker(Vector2(0, 0), 0) {
    recipe = RecipeInfo::convert_readable_recipe_into_recipe(
        {{}, 
        {{"grain", 1}}}, 
        {{peasant, 10}}
    );
}

SubsistenceFarm::SubsistenceFarm(Vector2i p_location, int p_owner): IsolatedBroker(p_location, p_owner) {
    recipe = RecipeInfo::convert_readable_recipe_into_recipe(
        {{}, 
        {{"grain", 1}}}, 
        {{peasant, 10}}
    );
}

std::unique_ptr<Recipe> SubsistenceFarm::get_recipe() {
    std::unique_ptr<Recipe> toReturn = std::unique_ptr<Recipe>(RecipeInfo::convert_readable_recipe_into_recipe(
        {{}, 
        {{"grain", 1}}}, 
        {{peasant, 10}}
    ));
    return toReturn;
}

void SubsistenceFarm::month_tick() {
    IsolatedBroker::month_tick();
}