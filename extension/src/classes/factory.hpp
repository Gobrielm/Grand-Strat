#pragma once

#include "factory_template.hpp"

using namespace godot;

class Factory : public FactoryTemplate {
    GDCLASS(Factory, FactoryTemplate)

protected:
    static void _bind_methods();

public:
    Factory();
    ~Factory();
    Factory(Vector2i new_location, int player_owner, Dictionary new_inputs, Dictionary new_outputs);

    static Terminal* create(Vector2i new_location, int player_owner, Dictionary new_inputs, Dictionary new_outputs);

    virtual void initialize(Vector2i new_location, int player_owner, Dictionary new_inputs, Dictionary new_outputs);

    // Recipe
    bool check_recipe();
    bool check_inputs();
    bool check_outputs();

    // Process Hooks
    virtual void day_tick() override;
    virtual void month_tick() override;
};
