#include "recipe_info.hpp"
#include "cargo_info.hpp"

#include "classes/factory_utility/recipe.hpp"
#include "classes/components/employer_component.hpp"

RecipeInfo* RecipeInfo::singleton_instance = nullptr;

RecipeInfo::RecipeInfo() {
    create_employer_components();
}

RecipeInfo::~RecipeInfo() {
    
}

void RecipeInfo::create() {
    if (singleton_instance == nullptr) {
        singleton_instance = (new(RecipeInfo));
    }
    
}

void RecipeInfo::cleanup() {
    if (singleton_instance != nullptr) {
        delete singleton_instance;
        singleton_instance = nullptr;
    }
}

RecipeInfo* RecipeInfo::get_instance() {
    ERR_FAIL_COND_V_MSG(singleton_instance == nullptr, nullptr, "RecipeInfo not created but accessed.");
    return singleton_instance;
}

EmployerComponent RecipeInfo::create_employer_component(std::vector<std::unordered_map<std::string, float>> v, std::unordered_map<PopTypes, int> p) {
    Ref<CargoInfo> cargo_info = CargoInfo::get_instance();
    std::vector<std::unordered_map<int, float>> v_float;
    v_float.emplace_back(); v_float.emplace_back(); // Add two maps to the vector for i/o
    for (const auto &[cargo_name, amount]: v[0]) {
        v_float[0][cargo_info->get_cargo_type(cargo_name.c_str())] = amount;
    }
    for (const auto &[cargo_name, amount]: v[1]) {
        v_float[1][cargo_info->get_cargo_type(cargo_name.c_str())] = amount;
    }
    return EmployerComponent(Recipe(v_float[0], v_float[1]), p);
}

void RecipeInfo::create_employer_components() {
    add_employer_component({{}, {{"clay", 0.05f}}}, {{PopTypes::rural, 2}});
    add_employer_component({{}, {{"sand", 0.05f}}}, {{PopTypes::rural, 2}});
    add_employer_component({{}, {{"sulfur", 0.05f}}}, {{PopTypes::rural, 2}});
    add_employer_component({{}, {{"lead", 0.05f}}}, {{PopTypes::rural, 2}});
    add_employer_component({{}, {{"iron", 0.05f}}}, {{PopTypes::rural, 2}});
    add_employer_component({{}, {{"coal", 0.05f}}}, {{PopTypes::rural, 2}});
    add_employer_component({{}, {{"copper", 0.05f}}}, {{PopTypes::rural, 2}});
    add_employer_component({{}, {{"zinc", 0.05f}}}, {{PopTypes::rural, 2}});
    add_employer_component({{}, {{"wood", 0.1f}}}, {{PopTypes::rural, 2}});
    add_employer_component({{}, {{"salt", 0.05f}}}, {{PopTypes::rural, 2}});
    add_employer_component({{}, {{"grain", 0.1f}}}, {{PopTypes::rural, 2}});
    add_employer_component({{}, {{"livestock", 0.1f}}}, {{PopTypes::rural, 2}});
    add_employer_component({{}, {{"fish", 0.1f}}}, {{PopTypes::rural, 2}});
    add_employer_component({{}, {{"fruit", 0.05f}}}, {{PopTypes::rural, 2}});
    add_employer_component({{}, {{"cotton", 0.05f}}}, {{PopTypes::rural, 2}});
    add_employer_component({{}, {{"silk", 0.05f}}}, {{PopTypes::rural, 2}});
    add_employer_component({{}, {{"spices", 0.05f}}}, {{PopTypes::rural, 2}});
    add_employer_component({{}, {{"coffee", 0.05f}}}, {{PopTypes::rural, 2}});
    add_employer_component({{}, {{"tea", 0.05f}}}, {{PopTypes::rural, 2}});
    add_employer_component({{}, {{"tobacco", 0.05f}}}, {{PopTypes::rural, 2}});
    add_employer_component({{}, {{"gold", 0.005f}}}, {{PopTypes::rural, 1}});

    // Secondary
    add_employer_component({{{"grain", 1.0f}, {"salt", 0.4f}}, {{"bread", 0.4f}}}, {{PopTypes::town, 2}});
    add_employer_component({{{"cotton", 1.0f}}, {{"clothes", 0.2f}}}, {{PopTypes::town, 2}});
    add_employer_component({{{"wood", 1.0f}}, {{"lumber", 0.3f}}}, {{PopTypes::town, 1}});
    add_employer_component({{{"wood", 1.0f}}, {{"paper", 0.5f}}}, {{PopTypes::town, 1}});
    add_employer_component({{{"lumber", 1.0f}}, {{"furniture", 0.3f}}}, {{PopTypes::town, 2}});
    add_employer_component({{{"lumber", 1.0f}}, {{"wagons", 0.25f}}}, {{PopTypes::town, 2}});
    add_employer_component({{{"lumber", 2.0f}}, {{"boats", 0.2f}}}, {{PopTypes::town, 3}});
}


void RecipeInfo::add_employer_component(std::pair<std::unordered_map<std::string, float>, std::unordered_map<std::string, float>> v, std::unordered_map<PopTypes, int> p) {
    Ref<CargoInfo> cargo_info = CargoInfo::get_instance();
    std::vector<std::unordered_map<int, float>> v_float;
    v_float.emplace_back(); v_float.emplace_back(); // Add two maps to the vector for i/o
    for (const auto &[cargo_name, amount]: v.first) {
        v_float[0][cargo_info->get_cargo_type(cargo_name.c_str())] = amount;
    }
    for (const auto &[cargo_name, amount]: v.second) {
        v_float[1][cargo_info->get_cargo_type(cargo_name.c_str())] = amount;
    }

    add_employer_component(EmployerComponent(Recipe(v_float[0], v_float[1]), p));
}

void RecipeInfo::add_employer_component(EmployerComponent employer_component) {
    employer_components.push_back(employer_component);
}

std::optional<EmployerComponent> RecipeInfo::get_primary_employer_component_for_type(int output_type) const {
    for (int i = 0; i < employer_components.size(); i++) {
        const auto &ec = employer_components[i];
        if (ec.recipe.is_primary() && ec.recipe.does_create(output_type)) {
            return ec;
        }
    }
    return std::nullopt;
}

std::optional<EmployerComponent> RecipeInfo::get_employer_component_for_type(int output_type) const {
    for (int i = 0; i < employer_components.size(); i++) {
        const auto &ec = employer_components[i];
        if (ec.recipe.does_create(output_type)) {
            return ec;
        }
    }
    return std::nullopt;
}

std::optional<EmployerComponent> RecipeInfo::get_employer_component(std::unordered_set<std::string> inputs, std::unordered_set<std::string> outputs) {
    for (int i = 0; i < employer_components.size(); i++) {
        const auto &ec = employer_components[i];
        if (map_and_set_match(inputs, ec.recipe.get_inputs()) && map_and_set_match(outputs, ec.recipe.get_outputs())) {
            return ec;
        }
    }
    return std::nullopt;
}

std::optional<EmployerComponent> RecipeInfo::get_employer_component(Dictionary inputs, Dictionary outputs) {
    for (int i = 0; i < employer_components.size(); i++) {
        const auto &ec = employer_components[i];
        if (map_and_dict_match(inputs, ec.recipe.get_inputs()) && map_and_dict_match(outputs, ec.recipe.get_outputs())) {
            return ec;
        }
    }
    return std::nullopt;
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