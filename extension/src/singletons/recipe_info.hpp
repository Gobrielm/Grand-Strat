#pragma once

#include <unordered_map>
#include <mutex>
#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/variant/dictionary.hpp>
#include <godot_cpp/variant/array.hpp>
#include "../classes/factory_utility/recipe.hpp"

using namespace godot;

class Recipe;

class RecipeInfo : public RefCounted {
    GDCLASS(RecipeInfo, RefCounted);

private:


protected:
    static void _bind_methods();
    static Ref<RecipeInfo> singleton_instance;
    std::vector<Recipe*> recipes;
public:
    RecipeInfo();
    
    static void create();
    static Ref<RecipeInfo> get_instance();
    void add_recipes();
    void create_recipe(std::vector<std::unordered_map<std::string, int>> v, std::unordered_map<PopTypes, int> p);
    void add_recipe(Recipe* recipe);
    Recipe* get_primary_recipe_for_type(int type) const;
    Recipe* get_recipe(Dictionary inputs, Dictionary outputs);
    bool map_and_dict_match(Dictionary d, std::unordered_map<int, int> m);
};
