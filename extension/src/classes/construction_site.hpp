#pragma once

#include <unordered_map>

#include "factory_template.hpp"

using namespace godot;

class ConstructionSite : public FactoryTemplate {
    GDCLASS(ConstructionSite, FactoryTemplate)

private:

    std::unordered_map<int, int> construction_materials;
    std::unordered_map<int, int> max_amounts;

protected:
    static void _bind_methods();

public:
    ConstructionSite();
    virtual ~ConstructionSite();
    ConstructionSite(Vector2i new_location, int player_owner);

    static Ref<ConstructionSite> create(Vector2i new_location, int player_owner);

    virtual void initialize(Vector2i new_location, int player_owner);

    //Recipe
    void set_recipe(const Array recipe);
    void destroy_recipe();
    Array get_recipe() const;
    bool has_recipe() const;

    //Materials
    void create_construction_materials();
    void create_construction_material(int type, int amount);
    Dictionary get_construction_materials() const;
    bool is_finished_constructing() const;

    // Process Hooks
    virtual void day_tick();
    virtual void month_tick();
};
