#pragma once

// #include <godot_cpp/core/binder_common.hpp>
// #include <godot_cpp/core/gdvirtual.gen.inc>

#include <unordered_map>
#include <vector>

#include "broker.hpp"
#include "factory_template.hpp"
#include "town_market.hpp"
#include "base_pop.hpp"

class CargoInfo;

using namespace godot;

class Town : public Broker {
    GDCLASS(Town, Broker)

private:
    TownMarket* market = nullptr;
    std::unordered_map<int, std::vector<FactoryTemplate*>> internal_factories;
    std::unordered_map<int, BasePop*> city_pops;

protected:
    static void _bind_methods();

public:
    Town();
    ~Town();
    Town(Vector2i new_location);

    static Terminal* create(Vector2i new_location);

    virtual void initialize(Vector2i new_location);

    // Trade
    bool does_accept(int type) const override;
    float get_local_price(int type) const override;
    Dictionary get_local_prices() const override;
    bool is_price_acceptable(int type, float price) const override;

    int get_desired_cargo(int type, float price) const override;
    int get_desired_cargo_from_train(int type) const override;
    void buy_cargo(int type, int amount, float price) override;
    int sell_cargo(int type, int amount, float price) override;

    void add_cash(float amount) override;
    void remove_cash(float amount) override;
    float get_cash() const override; 

    // Production
    float get_fulfillment(int type) const;
    Dictionary get_fulfillment_dict() const;
    void add_factory(FactoryTemplate* fact);
    Dictionary get_last_month_supply() const;
    Dictionary get_last_month_demand() const;

    //Storage
    Dictionary get_current_hold() const override;
    int add_cargo(int type, int amount) override;
    void remove_cargo(int type, int amount) override;
    int transfer_cargo(int type, int amount) override;

    //Pop stuff
    void add_pop(BasePop* pop);
    void sell_to_pops();
    void update_buy_orders();
    void sell_type(int type);
    int get_total_pops() const;
    FactoryTemplate* find_employment(BasePop* pop) const;

    //Selling to brokers
    void sell_to_other_brokers();
    void distribute_from_order(const TradeOrder* order) override;
    void report_attempt_to_sell(int type, int amount) override;
    std::vector<bool> get_accepts_vector() const override;

    // Process Hooks
    void day_tick();
    void month_tick();
};
