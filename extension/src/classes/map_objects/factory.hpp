#pragma once

#include "classes/factory_utility/recipe.hpp"
#include "classes/components/capital_component.hpp"
#include "classes/components/owner_component.hpp"
#include "classes/components/storage_component.hpp"
#include "classes/components/position_component.hpp"
#include "classes/components/construction_component.hpp"
#include "classes/components/employer_component.hpp"

#include "classes/trade_order.hpp"

#include <optional>

using namespace godot;

class Factory {

public:
    PositionComponent position;
    OwnerComponent owner;
    CapitalComponent capital;
    ConstructionComponent construction;
    StorageComponent storage;
    StorageComponent last_month_storage;
    // type -> true if bought/sold more than/equal to needed, false if sold/bought less than neccessary
    std::vector<int> storage_delta_indicator;

    EmployerComponent employer;
    LocalPriceController lpc;

    std::unordered_map<int, std::shared_ptr<TradeOrder>> orders; // Active orders sent to towns to buy/sell

    ~Factory();
    Factory(std::pair<int, int> p_position = {0, 0}, int p_owner = 0, EmployerComponent employer_component = EmployerComponent());

    Factory(const Factory& other);
    Factory& operator=(const Factory& other);

    //Construction
    void create_construction_materials();

    const Recipe& get_recipe() const;

    // Trade
    ///@brief Gets the minimum price seller would accept
    float get_min_price(int type, Town* town = nullptr) const; 
    ///@brief Gets the maximum price buyer would accept
    float get_max_price(int type, Town* town = nullptr) const; 
    float get_current_price(int type) const;
    bool does_create(int type) const;
    bool does_accept(int type) const;
    int get_desired_cargo(int type, float price_per);
    int get_desired_cargo_to_sell(int type);

    // Production
    void create_recipe();
    double get_batch_size();
    void remove_inputs(double batch_size);
    void add_outputs(double batch_size);
    String get_recipe_as_string() const;
    int get_primary_type() const;
    int get_supply(int type) const;
    int get_demand(int type) const;

    // Level & Upgrades
    void upgrade();
    void admin_upgrade();
    void finish_upgrade();
    bool is_primary_factory() const;
    float get_last_month_income() const;

    // Employment
    bool is_hiring(Town& town) const;
    bool is_hiring(PopTypes pop_type) const;
    bool is_firing(Town& town) const;
    float get_real_gross_profit(int months_to_average) const; // 1-indexed
    void employ_pop(Town& town, BasePop& pop);
    float get_wage(Town& town);

    // Process Hooks
    void month_tick();

    void adjust_trade_orders(Town& town);
};
