#pragma once

#include "factory_template.hpp"

using namespace godot;

class Factory : public FactoryTemplate {
    GDCLASS(Factory, FactoryTemplate)

protected:
    static void _bind_methods();

public:
    Factory();
    virtual ~Factory();
    Factory(Vector2i new_location, int player_owner, Recipe* p_recipe);

    // Recipe
    bool check_recipe();
    bool check_inputs();
    bool check_outputs();

    // Process Hooks
    virtual void day_tick() override;
    virtual void month_tick() override;
};
