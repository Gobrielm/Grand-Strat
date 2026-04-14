#pragma once

#include <unordered_map>
#include <mutex>
#include <godot_cpp/variant/dictionary.hpp>
#include <godot_cpp/variant/array.hpp>
#include "../classes/factory_utility/recipe.hpp"
#include <optional>

class Recipe;

class RecipeInfo {

private:


protected:
    static RecipeInfo* singleton_instance;
    std::vector<Recipe*> recipes;
public:
    RecipeInfo();
    ~RecipeInfo();
    
    static void create();
    static void cleanup();
    static RecipeInfo* get_instance();
    void add_recipes();
    void create_recipe(std::vector<std::unordered_map<std::string, float>> v, std::unordered_map<PopTypes, int> p);
    static Recipe convert_readable_recipe_into_recipe(std::vector<std::unordered_map<std::string, float>> v, std::unordered_map<PopTypes, int> p);
    void add_recipe(Recipe* recipe);
    std::optional<Recipe> get_primary_recipe_for_type(int output_type) const;
    /// @brief Will get the recipe that creates output type
    std::optional<Recipe> get_recipe_for_type(int output_type) const;
    std::optional<Recipe> get_recipe(std::unordered_set<std::string> inputs, std::unordered_set<std::string> outputs);
    std::optional<Recipe> get_recipe(Dictionary inputs, Dictionary outputs);
    bool map_and_dict_match(Dictionary d, std::unordered_map<int, float> m);
    bool map_and_set_match(std::unordered_set<std::string> s, std::unordered_map<int, float> m);
};
