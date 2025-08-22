#pragma once

#include <unordered_map>
#include "../base_pop.hpp"

using namespace godot;

class BasePop;

class Recipe {
    std::unordered_map<PopTypes, std::vector<int>> employees;
    std::unordered_map<int, float> inputs;
    std::unordered_map<int, float> outputs;
    std::unordered_map<PopTypes, int> pops_needed;

    protected:
    bool does_need_pop_type(PopTypes pop_type) const;
    void remove_pop(int pop_id, PopTypes pop_type);

    public:
    int level;

    Recipe();
    Recipe(std::unordered_map<int, float> p_inputs, std::unordered_map<int, float> p_outputs, std::unordered_map<PopTypes, int> p_pops_needed);
    Recipe(const Recipe& other);
    Dictionary get_inputs_dict() const;
    Dictionary get_outputs_dict() const;
    bool is_pop_type_needed(PopTypes pop_type) const;
    void add_pop(BasePop* pop);
    
    int get_employement() const;
    int get_pops_needed_num() const;
    float get_employment_rate() const;
    std::vector<int> fire_employees_and_get_vector();

    // Levels
    virtual void upgrade();
    virtual void degrade();
    virtual double get_level() const;
    virtual int get_level_without_employment() const;

    bool has_recipe() const;
    bool does_create(int type) const;
    bool is_primary() const;
    void clear();

    // Getters
    std::unordered_map<int, float> get_inputs() const;
    std::unordered_map<int, float> get_outputs() const;
    std::unordered_map<PopTypes, int> get_pops_needed() const;
    std::unordered_map<int, PopTypes> get_employee_ids() const;

    // Setters
    void set_inputs(const std::unordered_map<int, float> new_inputs);
    void set_outputs(const std::unordered_map<int, float> new_outputs);
    void set_pops_needed(const std::unordered_map<PopTypes, int> new_pops_needed);
};