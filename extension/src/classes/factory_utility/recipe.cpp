#include "recipe.hpp"
#include "singletons/cargo_info.hpp"

Recipe::Recipe() {
    level = 1;
}

Recipe::Recipe(std::unordered_map<int, float> p_inputs, std::unordered_map<int, float> p_outputs) {
    inputs = p_inputs;
    outputs = p_outputs;
    level = 1;
}

Recipe::Recipe(const Recipe& other) {
    inputs = other.inputs;
    outputs = other.outputs;
    level = other.level;
}

Dictionary Recipe::get_inputs_dict() const {
    Dictionary d;
    for (const auto &[type, amount]: inputs) {
        d[type] = amount;
    }
    return d;
}

Dictionary Recipe::get_outputs_dict() const {
    Dictionary d;
    for (const auto &[type, amount]: outputs) {
        d[type] = amount;
    }
    return d;
}

bool Recipe::has_recipe() const {
    return inputs.size() != 0 || outputs.size() != 0;
}

bool Recipe::does_create(int type) const {
    return outputs.count(type);
}
bool Recipe::is_primary() const {
    return inputs.size() == 0;
}

void Recipe::clear() {
    inputs.clear();
    outputs.clear();
}

float Recipe::get_input(int type) const {
    return 0 ? !inputs.count(type): inputs.at(type);
}

float Recipe::get_output(int type) const {
    return 0 ? !outputs.count(type): outputs.at(type);
}

std::unordered_map<int, float> Recipe::get_inputs() const {
    return inputs;
}

std::unordered_map<int, float> Recipe::get_outputs() const {
    return outputs;
}

void Recipe::set_inputs(const std::unordered_map<int, float> new_inputs) {
    inputs = new_inputs;
}

void Recipe::set_outputs(const std::unordered_map<int, float> new_outputs) {
    outputs = new_outputs;
}

String Recipe::get_recipe_as_string() const {
    Ref<CargoInfo> cargo_info = CargoInfo::get_instance();
    String x;
    int i = 0;
    for (const auto& [type, amount]: inputs) {
        x += String::num(amount) + " " + cargo_info->get_cargo_name(type);
        if (i < inputs.size() - 1) x += ", " ;
        i++;
    }
    if (inputs.size() != 0) x += " -> ";
    i = 0;
    for (const auto& [type, amount]: outputs) {
        x += String::num(amount) + " " + cargo_info->get_cargo_name(type);
        if (i < outputs.size() - 1) x += ", " ;
        i++;
    }
    return x;
}