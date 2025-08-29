#pragma once

#include <godot_cpp/classes/object.hpp>
#include "factory_template.hpp"
#include <unordered_map>
#include <mutex>

using namespace godot;

enum PopTypes {
    peasant = 0,
    rural = 1,
    town = 2,
    
    none = 99
};

class FactoryTemplate;

class BasePop {
    static std::unordered_map<PopTypes, int> PEOPLE_PER_POP;
    static std::unordered_map<PopTypes, int> INITIAL_WEALTH;
    static std::atomic<int> total_pops;
    static std::unordered_map<PopTypes, std::unordered_map<int, float>> base_needs;
    static std::unordered_map<PopTypes, std::unordered_map<int, float>> specialities;
    
    Vector2i location;
    const int pop_id;
    int education_level;
    float wealth;
    int home_prov_id;
    Variant culture;
    float income;
    int employement_id = -2;
    int months_starving = 0;
    int months_without_job = 0;

    PopTypes pop_type;
    std::unordered_map<int, float> internal_storage;
    
    protected:
    String _to_string() const;
    float get_buy_price_for_needed_good(int type, float current_price) const;
    float get_buy_price_for_wanted_good(int type, float current_price) const;
    bool are_needs_met() const;
    bool is_need_met(int type) const;
    bool is_want_met(int type) const;
    

    public:

    // Constructors
    static BasePop* create_rural_pop(int p_home_prov_id, Vector2i p_location, Variant p_culture);
    static BasePop* create_peasant_pop(int p_home_prov_id, Vector2i p_location, Variant p_culture);
    static BasePop* create_town_pop(int p_home_prov_id, Vector2i p_location, Variant p_culture);


    static void create_base_needs();
    static void create_base_wants();
    static std::unordered_map<PopTypes, std::unordered_map<int, float>> create_needs(std::string file_name);
    static int get_people_per_pop(PopTypes pop_type);
    int get_pop_id() const;
    void set_home_prov_id(int p_home_prov_id);
    int get_home_prov_id() const;
    void set_location(Vector2i p_location);
    Vector2i get_location() const;

    //Types
    void set_type(PopTypes p_pop_type);
    PopTypes get_type() const;
    static float get_base_need(PopTypes pop_type, int type);
    static float get_base_want(PopTypes pop_type, int type);
    static std::unordered_map<int, float> get_base_needs(PopTypes pop_type);
    static std::unordered_map<int, float> get_base_wants(PopTypes pop_type);
    std::unordered_map<int, float> get_base_needs() const;
    std::unordered_map<int, float> get_base_wants() const;

    bool is_seeking_employment() const;
    bool is_unemployed() const;
    void pay_wage(float wage);
    void employ(int p_employement_id, float wage);
    void fire();
    float get_income() const;
    bool is_wage_acceptable(float p_wage) const;
    float get_expected_income(std::unordered_map<int, float> current_prices) const;
    float get_sol() const;
    void add_wealth(double amount);

    bool is_starving() const;
    bool is_in_mild_starvation() const;
    bool is_in_medium_starvation() const;
    bool is_in_high_starvation() const;

    float get_base_need(int type) const;
    float get_base_want(int type) const;
    float get_buy_price(int type, float current_price) const;

    unsigned int get_desired(int type) const;
    unsigned int get_desired(int type, float price) const;
    void buy_good(int type, int amount, float price);
    void add_cargo(int type, int amount);
    int get_max_storage(int type) const;
    int get_education_level() const;
    float get_wealth() const;
    float transfer_wealth();
    Variant get_culture() const;
    float get_average_fulfillment() const;

    //Changing Pop Type
    bool will_degrade() const;
    void degrade();
    bool will_upgrade() const;
    void upgrade();
    void reset_and_fill_storage(); // Fills storage with needs, but just instaiates wants

    void month_tick();

    BasePop();
    BasePop(int p_home_prov_id, Vector2i p_location, Variant p_culture, PopTypes p_pop_type);
    BasePop& operator=(const BasePop& from) = delete;
    BasePop(const BasePop& other) = delete;
    BasePop(BasePop&&) noexcept = default;
    ~BasePop();
};