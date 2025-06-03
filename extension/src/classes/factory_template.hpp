#pragma once

#include <unordered_map>
#include <vector>
#include <list>

#include "broker.hpp"

using namespace godot;

class BasePop;

class FactoryTemplate : public Broker {
    GDCLASS(FactoryTemplate, Broker)

private:
    static const int COST_FOR_UPGRADE;
    static const int DEFAULT_BATCH_SIZE;

    int level = 1;
    int pops_needed = 1;
    float change_in_cash = 0.0;

    std::list<int> income_list;
    std::vector<BasePop*> employees;

protected:
    static void _bind_methods();

public:
    std::unordered_map<int, int> inputs;
    std::unordered_map<int, int> outputs;

    FactoryTemplate();
    ~FactoryTemplate();
    FactoryTemplate(Vector2i new_location, int player_owner, Dictionary new_inputs, Dictionary new_outputs);

    static Terminal* create(Vector2i new_location, int player_owner, Dictionary new_inputs, Dictionary new_outputs);

    virtual void initialize(Vector2i new_location, int player_owner, Dictionary new_inputs, Dictionary new_outputs);



    // Trade
    float get_min_price(int type) const;
    float get_max_price(int type) const;
    int get_monthly_demand(int type) const;
    int get_monthly_supply(int type) const;
    bool does_create(int type) const;

    // Production
    void create_recipe();
    int get_batch_size() const;
    void remove_inputs(int batch_size);
    void add_outputs(int batch_size);

    // Selling
    void distribute_cargo();

    // Level & Upgrades
    int get_level() const;
    int get_cost_for_upgrade() const;
    void check_for_upgrade();
    void upgrade();
    void admin_upgrade();
    void update_income_array();
    float get_last_month_income() const;

    // Employment
    int get_employement() const;
    bool is_hiring() const;
    bool is_firing() const;
    float get_wage() const;
    void work_here(BasePop* pop);
    void pay_employees();
    void fire_employees();

    // Process Hooks
    virtual void day_tick();
    virtual void month_tick();
};
