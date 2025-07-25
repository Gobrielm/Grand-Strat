#pragma once

#include <godot_cpp/classes/object.hpp>
#include "../base_pop.hpp"
#include <unordered_map>

using namespace godot;

enum PopTypes {
    rural = 1,
    town = 2,
    none = 99
};

class Recipe : public Object {
    GDCLASS(Recipe, Object);

    std::unordered_map<PopTypes, std::vector<BasePop*>> employees;
    std::unordered_map<int, int> inputs;
    std::unordered_map<int, int> outputs;
    std::unordered_map<PopTypes, int> pops_needed;

    protected:
    static void _bind_methods();
    PopTypes get_pop_type(BasePop* pop) const;
    bool does_need_pop_type(PopTypes pop_type) const;

    public:
    Recipe();
    Recipe(std::unordered_map<int, int> &p_inputs, std::unordered_map<int, int> &p_outputs, std::unordered_map<PopTypes, int> &p_pops_needed);
    Dictionary get_inputs_dict() const;
    Dictionary get_outputs_dict() const;
    bool is_pop_needed(BasePop* pop) const;
    void add_pop(BasePop* pop);


    
    // Getters
    std::unordered_map<int, int> get_inputs() const;
    std::unordered_map<int, int> get_outputs() const;
    std::unordered_map<PopTypes, int> get_pops_needed() const;

    // Setters
    void set_inputs(const std::unordered_map<int, int>& new_inputs);
    void set_outputs(const std::unordered_map<int, int>& new_outputs);
    void set_pops_needed(const std::unordered_map<PopTypes, int>& new_pops_needed);
};