#include "subsistence_farm.hpp"
#include "../../singletons/recipe_info.hpp"

void SubsistenceFarm::_bind_methods() {

}

SubsistenceFarm::SubsistenceFarm(int p_owner): IsolatedBroker(p_owner) {
    recipe = RecipeInfo::convert_readable_recipe_into_recipe(
        {{}, 
        {{"grain", 1}}}, 
        {{peasant, 30}}
    );
}