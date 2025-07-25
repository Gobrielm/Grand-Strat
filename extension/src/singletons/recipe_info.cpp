#include "recipe_info.hpp"
#include "cargo_info.hpp"

Ref<RecipeInfo> RecipeInfo::singleton_instance = nullptr;

void RecipeInfo::_bind_methods() {
    ClassDB::bind_static_method(get_class_static(), D_METHOD("get_instance"), &RecipeInfo::get_instance);

    ClassDB::bind_method(D_METHOD("month_tick"), &RecipeInfo::get_primary_recipe_for_type);
}

RecipeInfo::RecipeInfo() {
    add_recipes();
}

void RecipeInfo::create() {
    singleton_instance = Ref<RecipeInfo>(memnew(RecipeInfo));
}

Ref<RecipeInfo> RecipeInfo::get_instance() {
    return singleton_instance;
}

void RecipeInfo::add_recipes() {
    for (const auto &readable_recipe: readable_recipes) {
        Array recipe = convert_readable_recipe_to_usable(readable_recipe);
        add_recipe(recipe);
    }
}

Array RecipeInfo::convert_readable_recipe_to_usable(const std::vector<std::unordered_map<std::string, int>>& readable_recipe) {
    Ref<CargoInfo> cargo_info = CargoInfo::get_instance();
    Array toReturn;
    
    Dictionary new_inputs = convert_readable_side_of_recipe(readable_recipe[0]);
    toReturn.push_back(new_inputs);

    Dictionary new_outputs = convert_readable_side_of_recipe(readable_recipe[1]);
    toReturn.push_back(new_outputs);

    return toReturn;
}

Dictionary RecipeInfo::convert_readable_side_of_recipe(const std::unordered_map<std::string, int> &recipe_side) {
    Ref<CargoInfo> cargo_info = CargoInfo::get_instance();

    Dictionary new_side;
    for (const auto &[cargo_name, amount]: recipe_side) {
        new_side[cargo_info->get_cargo_type((cargo_name).c_str())] = amount;
    }
    return new_side;
}

void RecipeInfo::add_recipe(Array& recipe) {
    recipes.push_back(&recipe);
}

Array RecipeInfo::get_primary_recipe_for_type(int type) const {
    for (int i = 0; i < recipes.size(); i++) {
        Array recipe = recipes[i];
        if (Dictionary(recipe[1]).has(type)) {
            return recipe;
        }
    }
    return Array();
}