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
    static int total_pops;
    static std::mutex m;
    static std::unordered_map<int, float> base_needs;
    static std::unordered_map<int, float> specialities;
    

    int pop_id;
    int education_level;
    float wealth;
    int home_prov_id;
    Variant culture;
    float income;

    std::unordered_map<int, float> fulfillment;
    
    protected:
    static void _bind_methods();
    String _to_string() const;
    
    

    public:

    static void create_base_needs(Dictionary d);
    static int get_people_per_pop();
    int get_pop_id() const;
    void set_home_prov_id(int p_home_prov_id);
    int get_home_prov_id() const;
    void work_here(Ref<FactoryTemplate> work);
    bool is_seeking_employment() const;
    bool will_work_here(Ref<FactoryTemplate> fact) const;
    void pay_wage(float wage);
    void employ(float wage);
    void fire();
    float get_income() const;
    bool is_income_acceptable(float p_income) const;
    float get_expected_income() const;
    float get_sol() const;
    float get_desired(int type, float price) const;
    void buy_good(int type, float amount, float price);
    int get_education_level() const;
    float get_wealth() const;
    float transfer_wealth();
    Variant get_culture() const;
    float get_average_fulfillment() const;

    static BasePop* create(int p_home_prov_id = -1, Variant p_culture = 0);

    void initialize(int p_home_prov_id = -1, Variant p_culture = 0);
    

    BasePop();
    BasePop(int p_home_prov_id, Variant p_culture);
    ~BasePop();
};