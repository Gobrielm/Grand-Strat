#include "recipe.hpp"
#include "../Pops/rural_pop.hpp"
#include "../town_pop.hpp"

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

void Recipe::remove_pop(BasePop* pop) {
    int i = 0;
    std::vector<BasePop*> pop_v = employees[get_pop_type(pop)];
    for (auto it = pop_v.begin(); it != pop_v.end(); it++) {
        if (*it == pop) {
            employees[get_pop_type(pop)].erase(it);
        }
    }
}

int Recipe::get_employement() const {
    int total = 0;
    for (const auto [__, pop_vector]: employees) {
        total += pop_vector.size();
    }
    return total;
}

int Recipe::get_pops_needed_num() const {
    int total = 0;
    for (const auto [__, amount]: pops_needed) {
        total += amount;
    }
    return total;
}

void Recipe::fire_employees() {
    int fired = 0;
    int to_fire = std::max((int)(get_employement() * 0.1), 1); // Fire atleast 1 pop
    std::vector<BasePop*> pops = get_employees();
    while (fired < to_fire && pops.size() != 0) {
        int rand_index = rand() % pops.size();
        BasePop* pop = pops[rand_index];
        pops.erase(pops.begin() + rand_index); // Erase from local vector
        remove_pop(pop);                       // Erase from actual vector
        pop->fire();
        fired++;
    }
}

void Recipe::upgrade() {
    level++;
    for (const auto &[type, amount]: outputs) {
        outputs[type] = std::round(amount * (double(level) / (level - 1.0)));
    }
    for (const auto &[type, amount]: inputs) {
        inputs[type] = std::round(amount * (double(level) / (level - 1.0)));
    }
    for (const auto &[type, amount]: pops_needed) {
        pops_needed[type] = std::round(amount * (double(level) / (level - 1.0)));
    }
}

double Recipe::get_level() const {
	int employment = get_employement();
	if (employment == 0) {
        return 0;
    }
	return round(get_level_without_employment() * employment / get_pops_needed_num());
}

int Recipe::get_level_without_employment() const {
    return level;
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

std::vector<BasePop*> Recipe::get_employees() const {
    std::vector<BasePop*> v;
    for (const auto &[__, pop_vect]: employees) {
        for (const auto &pop: pop_vect) {
            v.push_back(pop);
        }
    }
    return v;
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