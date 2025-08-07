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
    create_recipe({{}, {{"clay", 1.0f}}}, {{rural, 1}});
    create_recipe({{}, {{"sand", 1.0f}}}, {{rural, 1}});
    create_recipe({{}, {{"sulfur", 1.0f}}}, {{rural, 1}});
    create_recipe({{}, {{"lead", 1.0f}}}, {{rural, 1}});
    create_recipe({{}, {{"iron", 1.0f}}}, {{rural, 1}});
    create_recipe({{}, {{"coal", 1.0f}}}, {{rural, 1}});
    create_recipe({{}, {{"copper", 1.0f}}}, {{rural, 1}});
    create_recipe({{}, {{"zinc", 1.0f}}}, {{rural, 1}});
    create_recipe({{}, {{"wood", 1.0f}}}, {{rural, 1}});
    create_recipe({{}, {{"salt", 1.0f}}}, {{rural, 1}});
    create_recipe({{}, {{"grain", 0.1f}}}, {{rural, 2}});
    create_recipe({{}, {{"livestock", 1.0f}}}, {{rural, 1}});
    create_recipe({{}, {{"fish", 1.0f}}}, {{rural, 1}});
    create_recipe({{}, {{"fruit", 1.0f}}}, {{rural, 1}});
    create_recipe({{}, {{"cotton", 1.0f}}}, {{rural, 1}});
    create_recipe({{}, {{"silk", 1.0f}}}, {{rural, 1}});
    create_recipe({{}, {{"spices", 1.0f}}}, {{rural, 1}});
    create_recipe({{}, {{"coffee", 1.0f}}}, {{rural, 1}});
    create_recipe({{}, {{"tea", 1.0f}}}, {{rural, 1}});
    create_recipe({{}, {{"tobacco", 1.0f}}}, {{rural, 1}});
    create_recipe({{}, {{"gold", 1.0f}}}, {{rural, 1}});

    // Secondary
    create_recipe({{{"grain", 5.0f}, {"salt", 2.0f}}, {{"bread", 2.0f}}}, {{town, 1}});
    create_recipe({{{"cotton", 5.0f}}, {{"clothes", 1.0f}}}, {{town, 1}});
    create_recipe({{{"wood", 3.0f}}, {{"lumber", 1.0f}}}, {{town, 1}});
    create_recipe({{{"wood", 2.0f}}, {{"paper", 1.0f}}}, {{town, 1}});
    create_recipe({{{"lumber", 3.0f}}, {{"furniture", 1.0f}}}, {{town, 1}});
    create_recipe({{{"lumber", 4.0f}}, {{"wagons", 1.0f}}}, {{town, 1}});
    create_recipe({{{"lumber", 10.0f}}, {{"boats", 1.0f}}}, {{town, 1}});
}


void RecipeInfo::create_recipe(std::vector<std::unordered_map<std::string, float>> v, std::unordered_map<PopTypes, int> p) {
    Ref<CargoInfo> cargo_info = CargoInfo::get_instance();
    std::vector<std::unordered_map<int, float>> v_float;
    v_float.emplace_back(); v_float.emplace_back(); // Add two maps to the vector for i/o
    for (const auto &[cargo_name, amount]: v[0]) {
        v_float[0][cargo_info->get_cargo_type(cargo_name.c_str())] = amount;
    }
    for (const auto &[cargo_name, amount]: v[1]) {
        v_float[1][cargo_info->get_cargo_type(cargo_name.c_str())] = amount;
    }

    add_recipe(new Recipe(v_float[0], v_float[1], p));
}

Recipe* RecipeInfo::convert_readable_recipe_into_recipe(std::vector<std::unordered_map<std::string, float>> v, std::unordered_map<PopTypes, int> p) {
    Ref<CargoInfo> cargo_info = CargoInfo::get_instance();
    std::vector<std::unordered_map<int, float>> v_float;
    v_float.emplace_back(); v_float.emplace_back(); // Add two maps to the vector for i/o
    for (const auto &[cargo_name, amount]: v[0]) {
        v_float[0][cargo_info->get_cargo_type(cargo_name.c_str())] = amount;
    }
    for (const auto &[cargo_name, amount]: v[1]) {
        v_float[1][cargo_info->get_cargo_type(cargo_name.c_str())] = amount;
    }
    return new Recipe(v_float[0], v_float[1], p);
}

void RecipeInfo::add_recipe(Recipe* recipe) {
    recipes.push_back(recipe);
}

Recipe* RecipeInfo::get_primary_recipe_for_type(int output_type) const {
    for (int i = 0; i < recipes.size(); i++) {
        Recipe* recipe = recipes[i];
        if (recipe->is_primary() && recipe->does_create(output_type)) {
            return new Recipe(*recipe);
        }
    }
    return nullptr;
}

Recipe* RecipeInfo::get_primary_recipe_for_type_read_only(int output_type) const {
    for (int i = 0; i < recipes.size(); i++) {
        Recipe* recipe = recipes[i];
        if (recipe->is_primary() && recipe->does_create(output_type)) {
            return recipe;
        }
    }
    return nullptr;
}

Recipe* RecipeInfo::get_secondary_recipe_for_type(int output_type) const {
    for (int i = 0; i < recipes.size(); i++) {
        Recipe* recipe = recipes[i];
        if (recipe->is_primary() && recipe->does_create(output_type)) {
            return new Recipe(*recipe);
        }
    }
    return nullptr;
}

Recipe* RecipeInfo::get_recipe(std::unordered_set<std::string> inputs, std::unordered_set<std::string> outputs) {
    for (int i = 0; i < recipes.size(); i++) {
        Recipe* recipe = recipes[i];
        if (map_and_set_match(inputs, recipe->get_inputs()) && map_and_set_match(outputs, recipe->get_outputs())) {
            return new Recipe(*recipe);
        }
    }
    return nullptr;
}

Recipe* RecipeInfo::get_recipe(Dictionary inputs, Dictionary outputs) {
    for (int i = 0; i < recipes.size(); i++) {
        Recipe* recipe = recipes[i];
        if (map_and_dict_match(inputs, recipe->get_inputs()) && map_and_dict_match(outputs, recipe->get_outputs())) {
            return new Recipe(*recipe);
        }
    }
    return nullptr;
}

bool RecipeInfo::map_and_dict_match(Dictionary d, std::unordered_map<int, float> m) {
    for (const auto& [type, amount]: m) {
        if (!d.has(type) || float(d[type]) != amount) {
            return false;
        }
    }
    return d.size() == m.size();
}

bool RecipeInfo::map_and_set_match(std::unordered_set<std::string> s, std::unordered_map<int, float> m) {
    Ref<CargoInfo> cargo_info = CargoInfo::get_instance();
    for (const auto& cargo_name: s) {
        if (!m.count(cargo_info->get_cargo_type(cargo_name.c_str()))) {
            return false;
        }
    }
    return s.size() == m.size();
}