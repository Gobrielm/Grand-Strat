#pragma once

#include <unordered_map>
#include <vector>
#include <list>
#include "broker.hpp"


using namespace godot;

class Recipe;
class BasePop;

class FactoryTemplate : public Broker {
    GDCLASS(FactoryTemplate, Broker)

private:
    static const int COST_FOR_UPGRADE;
    static const int DEFAULT_BATCH_SIZE;

    float change_in_cash = 0.0;

    std::list<int> income_list;

protected:
    Recipe* recipe = nullptr;

    static void _bind_methods();

    bool can_factory_upgrade() const;
    bool is_primary_factory() const;
    int get_primary_type_production() const;

public:

    FactoryTemplate();
    virtual ~FactoryTemplate();
    FactoryTemplate(Vector2i new_location, int player_owner, Recipe* recipe);

    virtual void initialize(Vector2i new_location, int player_owner, Recipe* recipe);

    // Trade

    // Used for min price to sell
    float get_min_price(int type) const; 
    // Used for max price to buy
    float get_max_price(int type) const; 
    bool does_create(int type) const;

    // Production
    std::unordered_map<int, float> get_outputs() const;
    std::unordered_map<int, float> get_inputs() const;
    void create_recipe();
    int get_batch_size() const;
    void remove_inputs(int batch_size);
    void add_outputs(int batch_size);
    String get_recipe_as_string() const;
    int get_primary_type() const;

    // Selling
    void distribute_cargo();

    // Level & Upgrades
    int get_level() const;
    int get_level_without_employment() const;
    bool is_max_level() const;
    static int get_cost_for_upgrade();
    void check_for_upgrade();
    void upgrade();
    void admin_upgrade();
    void update_income_array();
    float get_last_month_income() const;

    // Employment
    bool is_hiring(const BasePop* pop) const;
    bool is_firing() const;
    float get_wage() const;
    float get_theoretical_gross_profit() const;
    float get_real_gross_profit(int months_to_average) const; // 1-indexed
    void employ_pop(BasePop* pop);
    void pay_employees();
    void fire_employees();

    // Process Hooks
    virtual void day_tick();
    virtual void month_tick();
};
