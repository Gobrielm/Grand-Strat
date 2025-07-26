#include "recipe_info.hpp"
#include "cargo_info.hpp"
#include "../classes/factory_utility/recipe.hpp"

RecipeInfo* RecipeInfo::singleton_instance = nullptr;

RecipeInfo::RecipeInfo() {
    add_recipes();
}

void RecipeInfo::create() {
    singleton_instance = (new(RecipeInfo));
}

RecipeInfo* RecipeInfo::get_instance() {
    return singleton_instance;
}

void RecipeInfo::add_recipes() {
    create_recipe({{}, {{"clay", 1}}}, {{rural, 1}});
    create_recipe({{}, {{"sand", 1}}}, {{rural, 1}});
    create_recipe({{}, {{"sulfur", 1}}}, {{rural, 1}});
    create_recipe({{}, {{"lead", 1}}}, {{rural, 1}});
    create_recipe({{}, {{"iron", 1}}}, {{rural, 1}});
    create_recipe({{}, {{"coal", 1}}}, {{rural, 1}});
    create_recipe({{}, {{"copper", 1}}}, {{rural, 1}});
    create_recipe({{}, {{"zinc", 1}}}, {{rural, 1}});
    create_recipe({{}, {{"wood", 1}}}, {{rural, 1}});
    create_recipe({{}, {{"salt", 1}}}, {{rural, 1}});
    create_recipe({{}, {{"grain", 10}}}, {{rural, 10}});
    create_recipe({{}, {{"livestock", 1}}}, {{rural, 1}});
    create_recipe({{}, {{"fish", 1}}}, {{rural, 1}});
    create_recipe({{}, {{"fruit", 1}}}, {{rural, 1}});
    create_recipe({{}, {{"cotton", 1}}}, {{rural, 1}});
    create_recipe({{}, {{"silk", 1}}}, {{rural, 1}});
    create_recipe({{}, {{"spices", 1}}}, {{rural, 1}});
    create_recipe({{}, {{"coffee", 1}}}, {{rural, 1}});
    create_recipe({{}, {{"tea", 1}}}, {{rural, 1}});
    create_recipe({{}, {{"tobacco", 1}}}, {{rural, 1}});
    create_recipe({{}, {{"gold", 1}}}, {{rural, 1}});

    // Secondary
    create_recipe({{{"grain", 5}, {"salt", 2}}, {{"bread", 2}}}, {{town, 1}});
    create_recipe({{{"cotton", 5}}, {{"clothes", 1}}}, {{town, 1}});
    create_recipe({{{"wood", 3}}, {{"lumber", 1}}}, {{town, 1}});
    create_recipe({{{"wood", 2}}, {{"paper", 1}}}, {{town, 1}});
    create_recipe({{{"lumber", 3}}, {{"furniture", 1}}}, {{town, 1}});
    create_recipe({{{"lumber", 4}}, {{"wagons", 1}}}, {{town, 1}});
    create_recipe({{{"lumber", 10}}, {{"boats", 1}}}, {{town, 1}});
}


void RecipeInfo::create_recipe(std::vector<std::unordered_map<std::string, int>> v, std::unordered_map<PopTypes, int> p) {
    Ref<CargoInfo> cargo_info = CargoInfo::get_instance();
    std::vector<std::unordered_map<int, int>> v_int;
    v_int.emplace_back(); v_int.emplace_back(); // Add two maps to the vector for i/o
    for (const auto &[cargo_name, amount]: v[0]) {
        v_int[0][cargo_info->get_cargo_type(cargo_name.c_str())] = amount;
    }
    for (const auto &[cargo_name, amount]: v[1]) {
        v_int[1][cargo_info->get_cargo_type(cargo_name.c_str())] = amount;
    }

    add_recipe(memnew(Recipe(v_int[0], v_int[1], p)));
}

void RecipeInfo::add_recipe(Recipe* recipe) {
    recipes.push_back(recipe);
}

Recipe* RecipeInfo::get_primary_recipe_for_type(int type) const {
    for (int i = 0; i < recipes.size(); i++) {
        Recipe* recipe = recipes[i];
        if (recipe->is_primary() && recipe->does_create(type)) {
            return memnew(Recipe(*recipe));
        }
    }
    return nullptr;
}

Recipe* RecipeInfo::get_recipe(Dictionary inputs, Dictionary outputs) {
    for (int i = 0; i < recipes.size(); i++) {
        Recipe* recipe = recipes[i];
        if (map_and_dict_match(inputs, recipe->get_inputs()) && map_and_dict_match(outputs, recipe->get_outputs())) {
            return memnew(Recipe(*recipe));
        }
    }
    return nullptr;
}

bool RecipeInfo::map_and_dict_match(Dictionary d, std::unordered_map<int, int> m) {
    for (const auto& [type, amount]: m) {
        if (!d.has(type) || int(d[type]) != amount) {
            return false;
        }
        return true;
    }
    return false;
}