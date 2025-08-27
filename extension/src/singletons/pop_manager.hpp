#pragma once

#include <memory>
#include <unordered_map>
#include "../classes/base_pop.hpp"
#include "pop_manager_utility/pop_manager_thread_pool.hpp"
#include "../classes/factory_template.hpp"

class PopManager {
    friend class Province;
    static std::shared_ptr<PopManager> singleton_instance;
    mutable std::shared_mutex m; // Deals with datastructures
    mutable std::shared_mutex employment_mutex; // Deals with employment_options mutex
    std::unordered_map<int, BasePop*> pops;
    static int constexpr NUMBER_OF_POP_LOCKS = 4096;
    mutable std::vector<std::shared_mutex*> pop_locks; // Deals with individual pops
    PopManagerThreadPool* thread_pool = nullptr;

    int thread_month_tick_loader();
    int get_pop_mutex_number(int pop_id) const;
    std::shared_mutex* get_lock(int pop_id) const;
    std::shared_lock<std::shared_mutex> lock_pop_read(int pop_id) const;
    std::unique_lock<std::shared_mutex> lock_pop_write(int pop_id) const;
    BasePop* get_pop(int pop_id) const;
    int get_pop_country_id(BasePop* pop) const;
    void month_tick(std::vector<BasePop*>& pop_group);
    void sell_to_pops(std::vector<BasePop*>& pop_group);
    void create_pop_location_to_towns(std::vector<BasePop*>& pop_group, std::unordered_map<Vector2i, Vector2i, godot_helpers::Vector2iHasher>& location_to_nearest_town) const;
    void change_pop_unsafe(BasePop* pop);
    void find_employment_for_pops(std::vector<BasePop*>& pop_group);

    // Find Employement functions
    using employ_type = std::unordered_map<PopTypes, std::unordered_map<int, std::set<godot::Ref<FactoryTemplate>, FactoryTemplate::FactoryWageCompare>>>;
    employ_type employment_options; // PopType -> Country id -> set of available factories

    void find_employment_for_rural_pop(BasePop* pop);
    void find_employment_for_town_pop(BasePop* pop);
    void employment_finder_helper(BasePop* pop, PopTypes pop_type);
    void refresh_employment_sorted_by_wage();
    void refresh_rural_employment_sorted_by_wage();
    void refresh_town_employment_sorted_by_wage();
    void refresh_town_employment_sorted_by_wage_helper(int country_id, const Vector2i& tile, employ_type& local_employment_options);
    void add_local_employment_options(employ_type& local_employment_options);
    Ref<FactoryTemplate> get_first_employment_option(PopTypes pop_type, int country_id) const;
    void remove_first_employment_option(PopTypes pop_type, int country_id, const Ref<FactoryTemplate>& double_check);

    public:
    static void create();
    static void cleanup();
    PopManager();
    ~PopManager();
    static std::shared_ptr<PopManager> get_instance();

    void month_tick();

    /// Returns with pop id
    void set_pop_location(int pop_id, const Vector2i& location);
    int create_pop(Variant culture, const Vector2i& p_location, PopTypes p_pop_type);
    void pay_pop(int pop_id, float wage);
    void fire_pop(int pop_id);
    void sell_cargo_to_pop(int pop_id, int type, int amount, float price);
    void give_pop_cargo(int pop_id, int type, int amount);
    int get_pop_desired(int pop_id, int type, float price);
    void pay_pops(int num_to_pay, double for_each);

    //Economy stats
    float get_average_cash_of_pops() const;
    int get_number_of_broke_pops() const;
    int get_number_of_starving_pops() const;
    float get_unemployment_rate() const;
    float get_real_unemployment_rate() const;
    /// @brief Runs in n time where n is the total amount of pops.
    /// @return A vector who has the total quantity of each type of pop.
    std::unordered_map<PopTypes, int> get_pop_type_statistics() const;
};