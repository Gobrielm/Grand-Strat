#pragma once

#include <vector>
#include <shared_mutex>
#include <memory>
#include <unordered_map>
#include <set>
#include <mutex>
#include <unordered_set>

#include <classes/base_pop.hpp>

enum class PopStats {
    AveragePopWealth,
    NumOfStarvingPops,
    NumOfBrokePops,
    NumOfPeasants,
    UnemploymentRate,
    RealUnemploymentRate,
    NumUnemployed,
    NumRealUnemployed,
    TotalPopWealth
};

class EmployerComponent;
class Province;

class ProvincePopManager {
    friend Province;

    std::unordered_map<int, BasePop> pops;

    const BasePop* get_pop(int pop_id) const;
    BasePop* get_pop(int pop_id);
    int get_pop_country_id(BasePop* pop) const;
    // void create_pop_location_to_towns(std::vector<BasePop*>& pop_group, std::unordered_map<Vector2i, Vector2i, godot_helpers::Vector2iHasher>& location_to_nearest_town) const;
    // Vector2i get_town_tile(const BasePop* pop) const;
    // void change_pop_unsafe(BasePop* pop);
    void find_employment_for_pops(Province* province);

    // Find Employement functions
    using employ_type = std::unordered_map<
        PopTypes, std::unordered_map<
            int, std::set<
                std::pair<float, int>, 
                std::greater<std::pair<float, int>
                >
            >
        >
    >;

    // employ_type employment_options; // PopType -> Country id -> set of available factories

    // void employment_finder_helper(BasePop* pop, PopTypes pop_type);
    /// @return Returns whether the pop is employed or not
    // bool employment_for_potential_investor(BasePop* pop, int country_id); 
    /// @brief Currently Only takes into account profitabiltiy over entire country
    // int get_cargo_type_to_build(int country_id) const;
    float get_profit_of_ec(const EmployerComponent ec, const std::unordered_map<int, float>& average_prices) const;
    // void refresh_employment_sorted_by_wage();
    // void refresh_rural_employment_sorted_by_wage();
    // void refresh_town_employment_sorted_by_wage();
    // void refresh_town_employment_sorted_by_wage_helper(int country_id, const Vector2i& tile, employ_type& local_employment_options);
    // void add_local_employment_options(employ_type& local_employment_options);
    // FactoryTemplate::FactoryWageWrapper get_first_employment_option(PopTypes pop_type, int country_id) const;
    // void remove_first_employment_option(PopTypes pop_type, int country_id, const Ref<FactoryTemplate>& double_check);

    public:
    ProvincePopManager();
    ~ProvincePopManager();

    // order phase
    void adjust_pop_orders(class Town& town);
    // trading phase
    void pop_tick(Province* province);
    void sell_to_pops(Province* province);

    void set_pop_location(int pop_id, const Vector2i& location);
    /// @return Pop id
    int create_pop(PopTypes pop_type, Variant culture, Vector2i location, int province_id);
    void pay_pop(int pop_id, float wage);
    void fire_pop(int pop_id);
    void sell_cargo_to_pop(int pop_id, int type, int amount, float price);
    void give_pop_cargo(int pop_id, int type, int amount);
    int get_pop_desired(int pop_id, int type, float price);
    void pay_pops(int num_to_pay, double for_each);
    float get_expected_wage(int pop_id) const;

    //Economy stats
    

    std::unordered_map<PopStats, long>& get_pop_statistics() const;
    long get_total_wealth_of_pops() const;
    int get_number_of_broke_pops() const;
    int get_number_of_starving_pops() const;
    float get_unemployment_rate() const;
    float get_real_unemployment_rate() const;
    /// @brief Runs in n time where n is the total amount of pops.
    /// @return A vector who has the total quantity of each type of pop.
    std::unordered_map<PopTypes, long> get_pop_type_statistics() const;
};