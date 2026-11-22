#pragma once

#include <unordered_map>
#include <unordered_set>
#include <optional>
#include <vector>

#include <godot_cpp/classes/ref.hpp>
#include <godot_cpp/variant/dictionary.hpp>
#include <godot_cpp/variant/array.hpp>

#include "classes/factory_utility/recipe.hpp"


class Recipe;
class EmployerComponent;

class RecipeInfo {

private:


protected:
    static RecipeInfo* singleton_instance;
    std::vector<EmployerComponent> employer_components;

public:
    RecipeInfo();
    ~RecipeInfo();
    
    static void create();
    static void cleanup();
    static RecipeInfo* get_instance();

    static EmployerComponent create_employer_component(std::vector<std::unordered_map<std::string, float>> v, std::unordered_map<PopTypes, int> p);

    void create_employer_components();
    void add_employer_component(std::pair<std::unordered_map<std::string, float>, std::unordered_map<std::string, float>> v, std::unordered_map<PopTypes, int> p);
    void add_employer_component(EmployerComponent employer_component);
    
    std::optional<EmployerComponent> get_primary_employer_component_for_type(int output_type) const;
    /// @brief Will get the recipe that creates output type
    std::optional<EmployerComponent> get_employer_component_for_type(int output_type) const;
    std::optional<EmployerComponent> get_employer_component(std::unordered_set<std::string> inputs, std::unordered_set<std::string> outputs);
    std::optional<EmployerComponent> get_employer_component(Dictionary inputs, Dictionary outputs);
    bool map_and_dict_match(Dictionary d, std::unordered_map<int, float> m);
    bool map_and_set_match(std::unordered_set<std::string> s, std::unordered_map<int, float> m);
};
