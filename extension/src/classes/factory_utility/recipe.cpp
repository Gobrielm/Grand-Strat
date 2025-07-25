#include "recipe.hpp"
#include "../Pops/rural_pop.hpp"
#include "../town_pop.hpp"

void Recipe::_bind_methods() {

}

Recipe::Recipe() {

}

Recipe::Recipe(std::unordered_map<int, int> &p_inputs, std::unordered_map<int, int> &p_outputs, std::unordered_map<PopTypes, int> &p_pops_needed) {
    inputs = p_inputs;
    outputs = p_outputs;
    pops_needed = p_pops_needed;
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

bool Recipe::is_pop_needed(BasePop* pop) const {
    return does_need_pop_type(get_pop_type(pop));
}

PopTypes Recipe::get_pop_type(BasePop* pop) const {
    if (dynamic_cast<RuralPop*>(pop)) { // Checks pop is needed and if it has enough
        return rural;
    } else if (dynamic_cast<TownPop*>(pop)) {
        return town;
    }
    return none;
}

bool Recipe::does_need_pop_type(PopTypes pop_type) const {
    return pops_needed.count(pop_type) && pops_needed.at(pop_type) != employees.at(pop_type).size();
}

void Recipe::add_pop(BasePop* pop) {
    employees[get_pop_type(pop)].push_back(pop);
}

std::unordered_map<int, int> Recipe::get_inputs() const {
    return inputs;
}

std::unordered_map<int, int> Recipe::get_outputs() const {
    return outputs;
}

std::unordered_map<PopTypes, int> Recipe::get_pops_needed() const {
    return pops_needed;
}

void Recipe::set_inputs(const std::unordered_map<int, int> &new_inputs) {
    inputs = new_inputs;
}

void Recipe::set_outputs(const std::unordered_map<int, int> &new_outputs) {
    outputs = new_outputs;
}

void Recipe::set_pops_needed(const std::unordered_map<PopTypes, int> &new_pops_needed) {
    pops_needed = new_pops_needed;
}