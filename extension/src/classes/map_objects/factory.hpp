#pragma once

#include <godot_cpp/classes/ref_counted.hpp>
#include <classes/factory_utility/recipe.hpp>
#include <classes/components/capital_component.hpp>
#include <classes/components/owner_component.hpp>
#include <classes/components/storage_component.hpp>
#include <classes/components/position_component.hpp>
#include <classes/components/construction_component.hpp>

using namespace godot;

class Factory: public RefCounted {
    GDCLASS(Factory, RefCounted)

protected:
    static void _bind_methods();

public:
    PositionComponent position;
    OwnerComponent owner;
    CapitalComponent capital;
    ConstructionComponent construction;
    StorageComponent storage;

    Recipe recipe;
    LocalPriceController lpc;

    ~Factory();
    Factory(std::pair<int, int> p_position = {0, 0}, int p_owner = 0, Recipe p_recipe = Recipe());

    // Process Hooks
    void day_tick();
    void month_tick();

    //Construction
    void create_construction_materials();

    // Trade
    ///@brief Gets the minimum price seller would accept
    float get_min_price(int type) const; 
    ///@brief Gets the maximum price buyer would accept
    float get_max_price(int type) const; 
    bool does_create(int type) const;
    bool does_accept(int type) const;
    int get_desired_cargo(int type, float price_per);

    // Production
    void create_recipe();
    double get_batch_size();
    void remove_inputs(double batch_size);
    void add_outputs(double batch_size);
    String get_recipe_as_string() const;
    int get_primary_type() const;

    // Level & Upgrades
    void upgrade();
    void admin_upgrade();
    void finish_upgrade();
    bool is_primary_factory() const;
    float get_last_month_income() const;

    // Employment
    bool is_hiring() const;
    bool is_hiring(PopTypes pop_type) const;
    bool is_firing() const;
    float get_wage() const;
    float get_theoretical_gross_profit() const;
    float get_real_gross_profit(int months_to_average) const; // 1-indexed
    void employ_pop(BasePop& pop);

};
