#pragma once

#include <unordered_map>
#include <vector>

#include "broker.hpp"
#include "factory_template.hpp"
#include "base_pop.hpp"
#include "town_utility/town_cargo.hpp"

class CargoInfo;
struct PopSaleResult;

using namespace godot;

class Town : public Broker {
    GDCLASS(Town, Broker)

private:
    std::unordered_map<int, std::vector<Ref<FactoryTemplate>>> internal_factories; //Not thread safe
    std::unordered_map<int, BasePop*> city_pops;
    float INITIAL_CASH = 10000;

protected:
    static void _bind_methods();
    std::unordered_map<int, std::priority_queue<TownCargo*, std::vector<TownCargo*>, TownCargo::TownCargoPtrCompare>> market_storage;

public:
    Town();
    virtual ~Town();
    Town(Vector2i new_location);

    static Ref<Town> create(Vector2i new_location);

    virtual void initialize(Vector2i new_location);

    std::vector<int> get_supply() const;
    std::vector<int> get_demand() const;

    int get_supply(int type) const;
    int get_demand(int type) const;

    bool is_price_acceptable(int type, float price) const override;
    int get_desired_cargo(int type, float price) const override;

    // Production
    float get_fulfillment(int type) const;
    Dictionary get_fulfillment_dict() const;
    void add_factory(Ref<FactoryTemplate> fact);
    Array get_factories() const;
    Dictionary get_last_month_supply() const;
    Dictionary get_last_month_demand() const;

    //Pop stuff
    void add_pop(BasePop* pop);
    void sell_to_pops();
    void sell_type(int type);
    void sell_type_to_rural_pops(int type, PopSaleResult& current_sales);
    std::unordered_map<int, BasePop*> get_rural_pops() const;
    void sell_type_to_city_pops(int type, PopSaleResult& current_sales);
    void sell_type_to_pops(int type, PopSaleResult& current_sales, const std::unordered_map<int, BasePop*> &pops);
    int get_total_pops() const;
    Ref<FactoryTemplate> find_employment(BasePop* pop) const;
    int get_number_of_broke_pops() const;

    //Selling to brokers
    void sell_to_other_brokers();
    void distribute_from_order(const TradeOrder* order) override;
    std::vector<bool> get_accepts_vector() const override;

    //Storage Replacement
    void buy_cargo(int type, int amount, float price, Vector2i seller) override;

    //Economy Stats
    float get_total_wealth_of_pops();
    float get_needs_met_of_pops();

    void update_buy_orders();

    // Process Hooks
    void day_tick();
    void month_tick();
};
