#include "employer_component.hpp"
#include "classes/map_objects/town.hpp"

EmployerComponent::EmployerComponent() {

}

EmployerComponent::EmployerComponent(Recipe p_recipe, std::unordered_map<PopTypes, int> p_pops_needed) {
    recipe = p_recipe;
    pops_needed = p_pops_needed;
    for (const auto &[pop_type, __]: pops_needed) {
        employees[pop_type] = {};
    }
}

EmployerComponent::EmployerComponent(const EmployerComponent& other):
    recipe(other.recipe),
    pops_needed(other.pops_needed) {
}

EmployerComponent& EmployerComponent::operator=(const EmployerComponent& other) {
    if (this == &other) return *this;

    employees = other.employees;
    pops_needed = other.pops_needed;
    pops_to_fire = other.pops_to_fire;

    return *this;
}

void EmployerComponent::upgrade() {
    int level = ++recipe.level;
    for (const auto &[type, amount]: pops_needed) {
        pops_needed[type] = std::round((amount * level) / (level - 1.0));
    }
}

void EmployerComponent::degrade() {
    if (recipe.level == 1) {
        print_error("Downgrading a building a level 1");
        return;
    }
    int level = --recipe.level;
    for (const auto &[type, amount]: pops_needed) {
        pops_needed[type] = std::round((amount * level) / (level + 1));
    }
}

bool EmployerComponent::is_pop_type_needed(PopTypes pop_type) const {
    return does_need_pop_type(pop_type);
}

bool EmployerComponent::does_need_pop_type(PopTypes pop_type) const {
    return pops_needed.count(pop_type) && pops_needed.at(pop_type) != employees.at(pop_type).size();
}

void EmployerComponent::add_pop(BasePop* pop) {
    employees[pop->get_type()].push_back(pop->get_pop_id());
}

void EmployerComponent::add_pop(PopTypes pop_type, int pop_id) {
    employees[pop_type].push_back(pop_id);
}

void EmployerComponent::remove_pop(int pop_id, PopTypes pop_type) {
    auto &vec = employees[pop_type];
    int i = 0;
    for (auto id: vec) {
        if (id == pop_id) {
            vec.erase(std::next(vec.begin(), i));
        }
        i++;
    }
}

int EmployerComponent::get_employement() const {
    int total = 0;
    for (const auto [__, pop_vector]: employees) {
        total += pop_vector.size();
    }
    return total;
}

int EmployerComponent::get_pops_needed_num() const {
    int total = 0;
    for (const auto [__, amount]: pops_needed) {
        total += amount;
    }
    return total;
}

float EmployerComponent::get_employment_rate() const {
    int employement = get_employement(); // Seperated because both lock, idk
    return employement / float(get_pops_needed_num());
}

void EmployerComponent::queue_employees_to_be_fired() {
    int fired = 0;
    int to_fire = std::max((int)(get_employement() * 0.1), 1); // Fire atleast 1 pop
    std::unordered_map<int, PopTypes> pop_ids = get_employee_ids();
    while (fired < to_fire && !pop_ids.empty()) {
        int rand_index = rand() % pop_ids.size();

        auto it = std::next(pop_ids.begin(), rand_index);
        int pop_id = it->first; 

        pop_ids.erase(it);              //Remove locally
        remove_pop(pop_id, it->second); //Remove within object
        pops_to_fire.push_back(pop_id);            //Add to vector to return
        fired++;
    }
}

double EmployerComponent::get_level() const {
	int employment = get_employement();
	if (employment == 0) {
        return 0;
    }
	return get_level_without_employment() * double(employment) / get_pops_needed_num();
}

int EmployerComponent::get_level_without_employment() const {
    return recipe.level;
}

std::unordered_map<PopTypes, int> EmployerComponent::get_pops_needed() const {
    return pops_needed;
}

std::unordered_map<int, PopTypes> EmployerComponent::get_employee_ids() const {
    std::unordered_map<int, PopTypes> map;
    for (const auto &[pop_type, pop_vect]: employees) {
        for (const auto &pop: pop_vect) {
            map[pop] = pop_type;
        }
    }
    return map;
}

void EmployerComponent::set_pops_needed(const std::unordered_map<PopTypes, int> new_pops_needed) {
    pops_needed = new_pops_needed;
}

float EmployerComponent::get_wage(const Town& town, float current_cash) const {
    float gross_profit = std::min(float(get_theoretical_gross_profit(town)), current_cash);

    int pops_needed = get_pops_needed_num();

    if (pops_needed == 0) return 0;
    return (gross_profit) / pops_needed;
}

float EmployerComponent::get_theoretical_gross_profit(const Town& town) const {
    float available = 0;

    double effective_level = std::max(get_level(), 1.0);
    for (const auto &[type, amount]: recipe.get_inputs()) {
        available -= town.mp.get_price(type) * amount * effective_level;
    }
    for (const auto &[type, amount]: recipe.get_outputs()) {
        available += town.mp.get_price(type) * amount * effective_level;
    }
    available *= 30;
    return available;
}