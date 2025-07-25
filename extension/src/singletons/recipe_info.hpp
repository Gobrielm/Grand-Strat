#pragma once

#include <unordered_map>
#include <mutex>
#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/variant/dictionary.hpp>
#include <godot_cpp/variant/array.hpp>

using namespace godot;

class RecipeInfo : public RefCounted {
    GDCLASS(RecipeInfo, RefCounted);

private:


protected:
    static void _bind_methods();
    static Ref<RecipeInfo> singleton_instance;
    Array recipes; // recipes -> Array(inputs, outputs)
    std::vector<std::vector<std::unordered_map<std::string, int>>> readable_recipes = {
        {{{}}, {{"clay", 1}}},
        {{{}}, {{"sand", 1}}},
        {{{}}, {{"sulfur", 1}}},
        {{{}}, {{"lead", 1}}},
        {{{}}, {{"iron", 1}}},
        {{{}}, {{"coal", 1}}},
        {{{}}, {{"copper", 1}}},
        {{{}}, {{"zinc", 1}}},
        {{{}}, {{"wood", 1}}},
        {{{}}, {{"salt", 1}}},
        {{{}}, {{"grain", 1}}},
        {{{}}, {{"livestock", 1}}},
        {{{}}, {{"fish", 1}}},
        {{{}}, {{"fruit", 1}}},
        {{{}}, {{"cotton", 1}}},
        {{{}}, {{"silk", 1}}},
        {{{}}, {{"spices", 1}}},
        {{{}}, {{"coffee", 1}}},
        {{{}}, {{"tea", 1}}},
        {{{}}, {{"tobacco", 1}}},
        {{{}}, {{"gold", 1}}},

        // Secondary
        {{{"grain", 5}, {"salt", 2}}, {{"bread", 2}}},
        {{{"cotton", 5}}, {{"clothes", 1}}},
        {{{"wood", 3}}, {{"lumber", 1}}},
        {{{"wood", 2}}, {{"paper", 1}}},
        {{{"lumber", 3}}, {{"furniture", 1}}},
        {{{"lumber", 4}}, {{"wagons", 1}}},
        {{{"lumber", 10}}, {{"boats", 1}}},

    };
public:
    RecipeInfo();
    
    static void create();
    static Ref<RecipeInfo> get_instance();
    void add_recipes();
    Array convert_readable_recipe_to_usable(const std::vector<std::unordered_map<std::string, int>> &readable_recipe);
    Dictionary convert_readable_side_of_recipe(const std::unordered_map<std::string, int> &recipe_side);
    void add_recipe(Array& recipe);
    Array get_primary_recipe_for_type(int type) const;
};
