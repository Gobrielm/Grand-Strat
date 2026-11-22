#pragma once

#include <unordered_map>
#include <godot_cpp/variant/array.hpp>
#include <godot_cpp/variant/dictionary.hpp>

#include "factory_template.hpp"

using namespace godot;

class ConstructionSite : public FactoryTemplate {
    GDCLASS(ConstructionSite, FactoryTemplate)

private:

protected:
    static void _bind_methods();

public:
    ConstructionSite();
    virtual ~ConstructionSite();
    ConstructionSite(Vector2i new_location, int player_owner);

    static Ref<ConstructionSite> create(Vector2i new_location, int player_owner);

    virtual void initialize(Vector2i new_location, int player_owner);

    //Recipe
    void set_recipe(Recipe* p_recipe);
    void destroy_recipe();
    Array get_recipe() const;
    bool has_recipe() const;

    //Materials
    float get_max_storage() const override;
    float get_max_storage(int type) const override;
    float get_max_storage_unsafe(int type) const override;

    //Overrides
    bool is_price_acceptable(int type, float pricePer) const override;
    int get_desired_cargo_from_train(int type, float pricePer) const;

    // Process Hooks
    virtual void day_tick() override;
    virtual void month_tick() override;
};
