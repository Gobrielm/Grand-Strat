#pragma once

#include "fixed_hold.hpp"
#include "trade_order.hpp"
#include "road_depot.hpp"

#include "local_price_controller.hpp"
#include "../utility/vector2i_hash.hpp"
#include "broker_utility/trade_interaction_compare.hpp"
#include <unordered_map>
#include <set>
#include <unordered_set>
#include <memory>

class TownCargo;
struct TradeInteraction;

using namespace godot;

class Broker : public FixedHold {
    GDCLASS(Broker, FixedHold);
    
    LocalPriceController* get_local_pricer() const;

    protected:
    static void _bind_methods();
    std::unordered_map<int, TradeOrder*> trade_orders;
    float change_in_cash = 0.0f; // Represents change in cash from selling and buying goods, does not account for taxes, or salaries
    std::unordered_set<Vector2i, godot_helpers::Vector2iHasher> connected_brokers;
    std::unordered_set<Vector2i, godot_helpers::Vector2iHasher> connected_stations;
    LocalPriceController* local_pricer = nullptr;
    const float MAX_TRADE_MARGIN = 1.05f;

    virtual std::set<TradeInteraction*, TradeInteractionPtrCompare> get_brokers_to_distribute_to(int type); // Abstract
    float get_price_average(int type, Ref<Broker> other) const;
    void add_broker_to_sorted_set(int type, std::unordered_set<int> &s, std::set<TradeInteraction*, TradeInteractionPtrCompare> &trade_interactions, TradeInteraction* trade_interaction);
    public:
    
    Broker();
    Broker(const Vector2i new_location, const int player_owner, const int p_max_amount = DEFAULT_MAX_STORAGE);
    virtual ~Broker();
    virtual void initialize(const Vector2i new_location, const int player_owner, const int p_max_amount = DEFAULT_MAX_STORAGE);
    Broker(const Broker&) = delete;

    bool can_afford(float price) const;
    bool can_afford_unsafe(float price) const;
    virtual Dictionary get_local_prices() const;
    virtual float get_local_price(int type) const;
    virtual float get_local_price_unsafe(int type) const;

    virtual int get_desired_cargo(int type, float pricePer) const;
    virtual int get_desired_cargo_unsafe(int type, float pricePer) const;

    virtual bool is_price_acceptable(int type, float pricePer) const;

    // Buy Cargo deals with paying owner and dealing with fees
    virtual void buy_cargo(int type, int amount, float price, int p_terminal_id);
    virtual void buy_cargo(const TownCargo* cargo);

    int sell_cargo(int type, int amount);
    float add_cargo(int type, float amount) override;
    void report_change_in_cash(float amount);

    virtual void place_order(int type, int amount, bool buy, float maxPrice);
    virtual void edit_order(int type, int amount, bool buy, float maxPrice);

    TradeOrder* get_order(int type) const;
    std::unordered_map<int, TradeOrder*> get_orders();
    Dictionary get_orders_dict();
    virtual void remove_order(int type);

    virtual void add_connected_broker(Ref<Broker> broker);
    virtual void remove_connected_broker(const Ref<Broker> broker);
    Dictionary get_connected_broker_locations();

    void add_connected_station(const Vector2i p_location);
    void remove_connected_station(const Vector2i p_location);

    int get_number_of_connected_terminals() const;

    virtual void distribute_cargo(); // abstract
    virtual void distribute_type(int type);
    
    virtual void distribute_type_to_broker(int type, Ref<Broker> otherBroker, Ref<RoadDepot> road_depot = nullptr);

    void survey_broad_market(int type);
    void survey_broker_market(int type, Ref<Broker> broker);

    void add_surveyed_demand(int type, float price, float amount);
    void add_surveyed_supply(int type, float price, float amount);

    void add_surveyed_demand_unsafe(int type, float price, float amount);
    void add_surveyed_supply_unsafe(int type, float price, float amount);

    // Returns demand - supply
    virtual float get_diff_between_demand_and_supply(int type) const; // TEMP VIRTUAL
    virtual float get_diff_between_demand_and_supply_unsafe(int type) const;
    virtual float get_demand_at_price_unsafe(int type, float price) const;

    Dictionary get_last_month_supply() const;
    Dictionary get_last_month_demand() const;
};
