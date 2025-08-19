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

using namespace godot;

class Town : public Broker {
    GDCLASS(Town, Broker)

private:
    std::unordered_map<int, std::vector<Ref<FactoryTemplate>>> internal_factories; //Not thread safe
    mutable std::mutex internal_factories_mutex;
    std::unordered_set<int> town_pop_ids;
    float INITIAL_CASH = 10000;

protected:
    static void _bind_methods();
    //TODO: Integrate with new local_pricer, TEMPORARY
    std::unordered_map<int, int> local_demand; // TEMP
    std::unordered_map<int, int> old_local_demand; // TEMP
    std::unordered_map<int, std::unordered_map<int, int>> cargo_sold_map; // type -> price * 10 -> amount
    std::unordered_map<int, std::multiset<TownCargo*, TownCargo::TownCargoPtrCompare>> cargo_sell_orders; // Lowest price first
    std::unordered_map<int, std::unordered_map<int, TownCargo*>> town_cargo_tracker; // Owner id -> type -> TownCargo*
    std::unordered_map<int, float> current_prices; // Keep track of prices from last month
    std::unordered_map<int, int> current_totals; // Keeps track of current totals of goods

    

    std::set<TradeInteraction*, TradeInteractionPtrCompare> get_brokers_to_distribute_to(int type) override;

    std::multiset<TownCargo *, TownCargo::TownCargoPtrCompare>::iterator delete_town_cargo(std::multiset<TownCargo *, TownCargo::TownCargoPtrCompare>::iterator &sell_order_it);

    void encode_cargo(TownCargo* town_cargo);
    void encode_existing_cargo(TownCargo* existing_town_cargo, const TownCargo* new_town_cargo);

    void update_local_price(int type);
    double get_weighted_average(std::unordered_map<int, int> &m) const;
    double get_weighted_average(std::multiset<TownCargo *, TownCargo::TownCargoPtrCompare> &s) const;
    
    

public:
    Town();
    virtual ~Town();
    Town(Vector2i new_location);

    static Ref<Town> create(Vector2i new_location);

    virtual void initialize(Vector2i new_location);

    std::vector<float> get_supply() const;
    std::vector<float> get_demand() const;
    

    float get_supply(int type) const;
    float get_demand(int type) const;

    bool is_price_acceptable(int type, float price) const override;
    int get_desired_cargo(int type, float price) const override;
    int get_desired_cargo_unsafe(int type, float price) const override;

    float get_cargo_amount(int type) const override;

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
    float get_local_price(int type) const override;
    float get_local_price_unsafe(int type) const override;
    Dictionary get_local_prices() const override;
    std::unordered_map<int, float> get_local_prices_map();

    //Storage Replacement
    void buy_cargo(int type, int amount, float price, int p_terminal_id) override;
    void buy_cargo(const TownCargo* cargo) override;
    float add_cargo(int type, float amount) override;
    void age_all_cargo();
    std::multiset<TownCargo *, TownCargo::TownCargoPtrCompare>::iterator return_cargo(std::multiset<TownCargo *, TownCargo::TownCargoPtrCompare>::iterator cargo_it, std::unordered_map<int, std::unordered_map<int, int>>& cargo_to_return);
    bool does_cargo_exist(int p_terminal_id, int type) const;

    void report_sale(int type, float price, float amount);

    void update_buy_orders();
    void update_local_prices();

    //TEMP INHERIT
    float get_diff_between_demand_and_supply(int type) const override;
    float get_diff_between_demand_and_supply_unsafe(int type) const override;
    int get_local_demand(int type) const; // TEMP BUT MAYBE NOT

    // Process Hooks
    void day_tick();
    void month_tick();
};
