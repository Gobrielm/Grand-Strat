#pragma once

#include <godot_cpp/classes/object.hpp>
#include "factory_template.hpp"
#include <unordered_map>
#include <mutex>

using namespace godot;

class FactoryTemplate;

class BasePop : public Object {
    GDCLASS(BasePop, Object);

    static constexpr int PEOPLE_PER_POP = 1000;
    static constexpr int INITIAL_WEALTH = 1000;
    static std::atomic<int> total_pops;
    static std::unordered_map<int, float> base_needs;
    static std::unordered_map<int, float> specialities;
    
    Vector2i location;
    const int pop_id;
    int education_level;
    float wealth;
    int home_prov_id;
    Variant culture;
    float income;
    int employement_id;

    std::unordered_map<int, float> internal_storage;
    
    protected:
    static void _bind_methods();
    String _to_string() const;
    float get_limit_price_for_wanted_good(int type) const;
    float get_limit_price_for_needed_good(int type) const;
    

    public:

    static void create_base_needs(Dictionary d);
    static int get_people_per_pop();
    int get_pop_id() const;
    void set_home_prov_id(int p_home_prov_id);
    int get_home_prov_id() const;
    void set_location(Vector2i p_location);
    Vector2i get_location() const;

    bool is_seeking_employment() const;
    void pay_wage(float wage);
    void employ(int p_employement_id, float wage);
    void fire();
    float get_income() const;
    bool is_income_acceptable(float p_income) const;
    float get_expected_income() const;
    float get_sol() const;

    int get_base_need(int type) const;
    int get_base_want(int type) const;
    float get_limit_price(int type) const;

    int get_desired(int type) const;
    int get_desired(int type, float price) const;
    void buy_good(int type, int amount, float price);
    int get_max_storage(int type) const;
    int get_education_level() const;
    float get_wealth() const;
    float transfer_wealth();
    Variant get_culture() const;
    float get_average_fulfillment() const;
    void month_tick();

    BasePop();
    BasePop(int p_home_prov_id, Vector2i p_location, Variant p_culture);
    ~BasePop();
};