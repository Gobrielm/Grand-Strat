#pragma once

#include <unordered_map>
#include <godot_cpp/classes/object.hpp>
#include "classes/base_pop.hpp"

using namespace godot;

class BasePop;

class Recipe {

    std::unordered_map<int, float> inputs;
    std::unordered_map<int, float> outputs;

    protected:

    public:
    int level;

    Recipe();
    Recipe(std::unordered_map<int, float> p_inputs, std::unordered_map<int, float> p_outputs);
    Recipe(const Recipe& other);
    Dictionary get_inputs_dict() const;
    Dictionary get_outputs_dict() const;

    bool has_recipe() const;
    bool does_create(int type) const;
    bool is_primary() const;
    void clear();

    // Getters

    float get_input(int type) const;
    float get_output(int type) const;

    std::unordered_map<int, float> get_inputs() const;
    std::unordered_map<int, float> get_outputs() const;

    // Setters
    void set_inputs(const std::unordered_map<int, float> new_inputs);
    void set_outputs(const std::unordered_map<int, float> new_outputs);
};