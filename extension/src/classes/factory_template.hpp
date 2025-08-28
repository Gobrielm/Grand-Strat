#pragma once

#include <unordered_map>
#include <vector>
#include <list>
#include <shared_mutex>
#include "broker.hpp"


using namespace godot;

class Recipe;
class BasePop;
enum PopTypes;

class FactoryTemplate : public Broker {
    GDCLASS(FactoryTemplate, Broker)

private:
    static const int COST_FOR_UPGRADE;
    static const int DEFAULT_BATCH_SIZE;

    float change_in_cash = 0.0;

    std::list<int> income_list;

protected:
    Recipe* recipe = nullptr;

    // For upgrading and creation
    std::unordered_map<int, float> construction_materials;
    std::unordered_map<int, float> max_amounts_of_construction_materials;

    static void _bind_methods();

    bool can_factory_upgrade() const;
    bool is_primary_factory() const;
    int get_primary_type_production() const;

public:

    FactoryTemplate();
    virtual ~FactoryTemplate();
    FactoryTemplate(Vector2i new_location, int player_owner, Recipe* recipe);

    virtual void initialize(Vector2i new_location, int player_owner, Recipe* recipe);

    //Construction
    void create_construction_materials();
    void create_construction_material(int type, int amount);
    Dictionary get_construction_materials() const;
    bool is_needed_for_construction(int type) const;
    bool is_needed_for_construction_unsafe(int type) const;
    int get_amount_of_type_needed_for_construction_unsafe(int type) const;
    bool is_finished_constructing() const;
    bool is_constructing() const;

    // Trade
    float add_cargo_ignore_accepts(int type, float amount) override;
    // Used for min price to sell
    float get_min_price(int type) const; 
    // Used for max price to buy
    float get_max_price(int type) const; 
    bool does_create(int type) const;
    bool does_accept(int type) const override;
    bool does_accept_unsafe(int type) const;
    int get_desired_cargo(int type, float price_per) const override;
    int get_desired_cargo_unsafe(int type, float price_per) const override;

    // Production
    std::unordered_map<int, float> get_outputs() const;
    std::unordered_map<int, float> get_inputs() const;
    void create_recipe();
    double get_batch_size() const;
    void remove_inputs(double batch_size);
    void add_outputs(double batch_size);
    String get_recipe_as_string() const;
    int get_primary_type() const;

    // Selling
    void distribute_cargo();

    // Level & Upgrades
    double get_level() const;
    int get_level_without_employment() const;
    bool is_max_level() const;
    void upgrade();
    void admin_upgrade();
    void finish_upgrade();
    void update_income_array();
    float get_last_month_income() const;

    // Employment
    bool is_hiring(PopTypes pop_type) const;
    bool is_firing() const;
    float get_wage() const;
    float get_wage_unsafe() const;
    float get_theoretical_gross_profit() const;
    float get_theoretical_gross_profit_unsafe() const;
    float get_real_gross_profit(int months_to_average) const; // 1-indexed
    void employ_pop(BasePop* pop, std::shared_mutex &pop_lock, PopTypes pop_type);
    void pay_employees();
    void fire_employees();

    // Process Hooks
    virtual void day_tick();
    virtual void month_tick();

    struct FactoryWageWrapper {
        Ref<FactoryTemplate> internal_fact;
        float wage;
        FactoryWageWrapper(Ref<FactoryTemplate> fact = nullptr) {
            internal_fact = fact;
            wage = internal_fact->get_wage();
        }

        struct FactoryWageCompare {
            bool operator()(const FactoryWageWrapper& a, const FactoryWageWrapper& b) const {
                float a_wage = a.wage;
                float b_wage = a.wage;
                if (a_wage == b_wage) {
                    return &a > &b;
                }
                return a_wage > b_wage; // highest wage first
            }
        };

    };

};
