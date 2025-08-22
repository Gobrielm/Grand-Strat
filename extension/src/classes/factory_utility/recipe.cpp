#include "recipe.hpp"

Recipe::Recipe() {
    level = 1;
}

Recipe::Recipe(std::unordered_map<int, float> p_inputs, std::unordered_map<int, float> p_outputs, std::unordered_map<PopTypes, int> p_pops_needed) {
    inputs = p_inputs;
    outputs = p_outputs;
    pops_needed = p_pops_needed;
    for (const auto &[pop_type, __]: pops_needed) {
        employees[pop_type] = {};
    }
    level = 1;
}

Recipe::Recipe(const Recipe& other) {
    inputs = other.get_inputs();
    outputs = other.get_outputs();
    pops_needed = other.get_pops_needed();
    employees = other.employees;
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

bool Recipe::is_pop_type_needed(PopTypes pop_type) const {
    return does_need_pop_type(pop_type);
}

bool Recipe::does_need_pop_type(PopTypes pop_type) const {
    return pops_needed.count(pop_type) && pops_needed.at(pop_type) != employees.at(pop_type).size();
}

void Recipe::add_pop(BasePop* pop) {
    employees[pop->get_type()].push_back(pop->get_pop_id());
}

void Recipe::remove_pop(int pop_id, PopTypes pop_type) {
    auto &vec = employees[pop_type];
    vec.erase(std::remove(vec.begin(), vec.end(), pop_id), vec.end());
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

float Recipe::get_employment_rate() const {
    int employement = get_employement(); // Seperated because both lock, idk
    return employement / float(get_pops_needed_num());
}

std::vector<int> Recipe::fire_employees_and_get_vector() {
    std::vector<int> v;
    int fired = 0;
    int to_fire = std::max((int)(get_employement() * 0.1), 1); // Fire atleast 1 pop
    std::unordered_map<int, PopTypes> pop_ids = get_employee_ids();
    while (fired < to_fire && !pop_ids.empty()) {
        int rand_index = rand() % pop_ids.size();

        auto it = std::next(pop_ids.begin(), rand_index);
        int pop_id = it->first; 

        pop_ids.erase(it);              //Remove locally
        remove_pop(pop_id, it->second); //Remove within object
        v.push_back(pop_id);            //Add to vector to return
        fired++;
    }
    return v;
}

void Recipe::upgrade() {
    level++;
    for (const auto &[type, amount]: pops_needed) {
        pops_needed[type] = std::round(amount * (double(level) / (level - 1.0)));
    }
}

void Recipe::degrade() {
    if (level == 1) {
        print_error("Downgrading a building a level 1");
        return;
    }
    level--;
    for (const auto &[type, amount]: pops_needed) {
        pops_needed[type] = std::round(amount * (double(level) / (level + 1.0)));
    }
}

double Recipe::get_level() const {
	int employment = get_employement();
	if (employment == 0) {
        return 0;
    }
	return get_level_without_employment() * employment / get_pops_needed_num();
}

int Recipe::get_level_without_employment() const {
    return level;
}

bool Recipe::has_recipe() const {
    return inputs.size() == 0 && outputs.size() == 0;
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
    pops_needed.clear();
}

std::unordered_map<int, float> Recipe::get_inputs() const {
    return inputs;
}

std::unordered_map<int, float> Recipe::get_outputs() const {
    return outputs;
}

std::unordered_map<PopTypes, int> Recipe::get_pops_needed() const {
    return pops_needed;
}

std::unordered_map<int, PopTypes> Recipe::get_employee_ids() const {
    std::unordered_map<int, PopTypes> map;
    for (const auto &[pop_type, pop_vect]: employees) {
        for (const auto &pop: pop_vect) {
            map[pop] = pop_type;
        }
    }
    return map;
}

void Recipe::set_inputs(const std::unordered_map<int, float> new_inputs) {
    inputs = new_inputs;
}

void Recipe::set_outputs(const std::unordered_map<int, float> new_outputs) {
    outputs = new_outputs;
}

void Recipe::set_pops_needed(const std::unordered_map<PopTypes, int> new_pops_needed) {
    pops_needed = new_pops_needed;
}