#pragma once

#include <memory>
#include <unordered_map>
#include "../classes/base_pop.hpp"
#include "../utility/thread_pool.hpp"
#include "../classes/factory_template.hpp"

class PopManager {
    static std::shared_ptr<PopManager> singleton_instance;
    mutable std::shared_mutex m; // Deals with datastructures
    std::unordered_map<int, BasePop*> pops;
    mutable std::vector<std::shared_mutex> pop_locks; // Deals with individual pops
    ThreadPool<BasePop*>* thread_pool = nullptr;

    void thread_month_tick_loader();
    std::shared_mutex* get_lock(int pop_id);
    std::shared_lock<std::shared_mutex> lock_pop_read(int pop_id);
    std::unique_lock<std::shared_mutex> lock_pop_write(int pop_id);
    BasePop* get_pop(int pop_id) const;
    int get_pop_country_id(BasePop* pop) const;
    void month_tick(BasePop* pop);
    void sell_to_pop(BasePop* pop);
    void change_pop(BasePop* pop);
    void find_employment_for_pop(BasePop* pop);

    // Find Employement functions
    std::unordered_map<int, std::set<Ref<FactoryTemplate>, FactoryTemplate::FactoryWageCompare>> rural_employment_options; // Country id -> set of available factories
    std::unordered_map<int, std::set<Ref<FactoryTemplate>, FactoryTemplate::FactoryWageCompare>> town_employment_options; // Country id -> set of available factories

    void find_employment_for_rural_pop(BasePop* pop);
    void find_employment_for_town_pop(BasePop* pop);
    void employment_finder_helper(BasePop* pop, std::set<godot::Ref<FactoryTemplate>, FactoryTemplate::FactoryWageCompare> &employment_options);
    void refresh_rural_employment_sorted_by_wage();
    void refresh_rural_employment_sorted_by_wage_helper(int country_id, const Vector2i& tile);
    void refresh_town_employment_sorted_by_wage();
    void refresh_town_employment_sorted_by_wage_helper(int country_id, const Vector2i& tile);

    public:
    static void create();
    PopManager();
    ~PopManager();
    static std::shared_ptr<PopManager> get_instance();

    void month_tick();

    // Pop Utility

    /// Returns with pop id
    int create_pop(Variant culture, const Vector2i& p_location, PopTypes p_pop_type);
    void pay_pop(int pop_id, float wage);
    void fire_pop(int pop_id);
    void sell_cargo_to_pop(int pop_id, int type, int amount, float price);
    void give_pop_cargo(int pop_id, int type, int amount);
    int get_pop_desired(int pop_id, int type, float price);
    void pay_pops(int num_to_pay, double for_each);
};