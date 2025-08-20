#pragma once

#include <unordered_map>
#include <set>
#include <vector>

#include "broker.hpp"
#include "factory_template.hpp"
#include "road_depot.hpp"
#include "base_pop.hpp"
#include "town_utility/town_cargo.hpp"
#include "town_utility/pop_order.hpp"

class CargoInfo;
class TownLocalPriceController;

using namespace godot;

class Town : public Broker {
    GDCLASS(Town, Broker)

private:
    std::unordered_map<int, std::vector<Ref<FactoryTemplate>>> internal_factories; //Not thread safe
    mutable std::mutex internal_factories_mutex;
    std::unordered_set<int> town_pop_ids;
    float INITIAL_CASH = 10000;

    TownLocalPriceController* get_local_pricer() const;
protected:
    static void _bind_methods();
    
    
    std::set<TradeInteraction*, TradeInteractionPtrCompare> get_brokers_to_distribute_to(int type) override;

public:
    Town();
    virtual ~Town();
    Town(Vector2i new_location);

    static Ref<Town> create(Vector2i new_location);

    virtual void initialize(Vector2i new_location);

    std::unordered_map<int, float> get_supply() const;
    std::unordered_map<int, float> get_demand() const;
    

    float get_supply(int type) const;
    float get_demand(int type) const;

    bool is_price_acceptable(int type, float price) const override;
    int get_desired_cargo(int type, float price) const override;
    int get_desired_cargo_unsafe(int type, float price) const override;

    // Production
    float get_fulfillment(int type) const;
    Dictionary get_fulfillment_dict() const;
    void add_factory(Ref<FactoryTemplate> fact);
    Array get_factories() const;

    //Pop stuff
    void add_pop(int pop_id);
    void sell_to_pop(BasePop* pop);
    void pay_factory(int amount, float price, Vector2i source);
    int get_total_pops() const;
    std::set<Ref<FactoryTemplate>, FactoryTemplate::FactoryWageCompare> get_employment_sorted_by_wage(PopTypes pop_type) const;

    //Selling to brokers
    void distribute_cargo() override;
    void distribute_type(int type) override;
    void distribute_type_to_broker(int type, Ref<Broker> broker, Ref<RoadDepot> road_depot = Ref<RoadDepot>(nullptr)) override;
    std::vector<bool> get_accepts_vector() const override;
    Dictionary get_local_prices() const override;
    std::unordered_map<int, float> get_local_prices_map();

    //Storage Replacement
    void buy_cargo(int type, int amount, float price, int p_terminal_id) override;
    void buy_cargo(const TownCargo* cargo) override;
    float add_cargo(int type, float amount) override;
    void age_all_cargo();

    void update_buy_orders();

    //TEMP INHERIT
    float get_diff_between_demand_and_supply(int type) const override;
    float get_diff_between_demand_and_supply_unsafe(int type) const override;
    int get_local_demand(int type) const; // TEMP BUT MAYBE NOT

    // Process Hooks
    void day_tick();
    void month_tick();
};
