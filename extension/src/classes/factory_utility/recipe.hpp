#pragma once

#include <unordered_map>
#include "../base_pop.hpp"

using namespace godot;

enum PopTypes {
    rural = 1,
    town = 2,
    none = 99
};

class BasePop;

class Recipe {
    std::unordered_map<PopTypes, std::vector<BasePop*>> employees;
    std::unordered_map<int, int> inputs;
    std::unordered_map<int, int> outputs;
    std::unordered_map<PopTypes, int> pops_needed;

    protected:
    PopTypes get_pop_type(const BasePop* pop) const;
    bool does_need_pop_type(PopTypes pop_type) const;

    public:
    int level = 1;

    Recipe();
    Recipe(std::unordered_map<int, int> &p_inputs, std::unordered_map<int, int> &p_outputs, std::unordered_map<PopTypes, int> &p_pops_needed);
    Recipe(const Recipe& other);
    Dictionary get_inputs_dict() const;
    Dictionary get_outputs_dict() const;
    bool is_pop_needed(const BasePop* pop) const;
    void add_pop(BasePop* pop);
    void remove_pop(BasePop* pop);
    int get_employement() const;
    int get_pops_needed_num() const;
    void fire_employees();

    // Levels
    void upgrade();
    double get_level() const;
    int get_level_without_employment() const;

    bool has_recipe() const;
    bool does_create(int type) const;
    bool is_primary() const;
    void clear();

    // Getters
    std::unordered_map<int, int> get_inputs() const;
    std::unordered_map<int, int> get_outputs() const;
    std::unordered_map<PopTypes, int> get_pops_needed() const;
    std::vector<BasePop*> get_employees() const;

    // Setters
    void set_inputs(const std::unordered_map<int, int>& new_inputs);
    void set_outputs(const std::unordered_map<int, int>& new_outputs);
    void set_pops_needed(const std::unordered_map<PopTypes, int>& new_pops_needed);
};