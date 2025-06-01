#pragma once

#include <godot_cpp/classes/object.hpp>
#include <unordered_map>

using namespace godot;

class BasePop : public Object {
    GDCLASS(BasePop, Object);

    static const int PEOPLE_PER_POP;
    static int total_pops;
    static std::unordered_map<int, float> base_needs;
    static std::unordered_map<int, float> specialities;
    static const int INITIAL_WEALTH;

    int pop_id;
    int education_level;
    float wealth;
    int home_prov_id;
    Variant culture;
    float income;
    
    protected:
    static void _bind_methods();
    String _to_string() const;
    

    public:

    static void create_base_needs(Dictionary d);
    static int get_people_per_pop();
    int get_pop_id() const;
    int get_home_prov_id() const;
    // void find_employment() const;
    bool is_seeking_employment() const;
    void pay_wage(float wage);
    void employ(float wage);
    void fire();
    float get_income() const;
    bool is_income_acceptable(float p_income) const;
    float get_expected_income() const;
    float get_sol() const;
    float get_desired(int type, float price) const;
    void buy_good(float amount, float price);
    int get_education_level() const;
    float get_wealth() const;
    float transfer_wealth();
    Variant get_culture() const;

    static BasePop* create(int p_home_prov_id = -1, Variant p_culture = 0);

    void initialize(int p_home_prov_id = -1, Variant p_culture = 0);
    

    _FORCE_INLINE_ BasePop(): BasePop(-1, 0) {}
    _FORCE_INLINE_ BasePop(int p_home_prov_id, Variant p_culture) {
        home_prov_id = p_home_prov_id;
        culture = p_culture;
        pop_id = total_pops++;
        wealth = INITIAL_WEALTH;
        income = 0.0;
        education_level = 0;
    }
    ~BasePop();
};