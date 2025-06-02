#pragma once

// #include <godot_cpp/core/binder_common.hpp>
// #include <godot_cpp/core/gdvirtual.gen.inc>

#include <unordered_map>
#include <vector>

#include "broker.hpp"
#include "factory_template.hpp"
#include "town_market.hpp"
#include "base_pop.hpp"

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

    void create_storage();

    // Trade
    bool does_accept(int type) const override;
    float get_local_price(int type) const override;
    bool is_price_acceptable(int type, float price) const override;

    int get_desired_cargo(int type, float price) const override;
    void buy_cargo(int type, int amount, float price) override;
    int sell_cargo(int type, int amount, float price) override;

    // Production
    float get_fulfillment(int type) const;
    Dictionary get_fulfillment_dict() const;
    void add_factory(FactoryTemplate* fact);

    //Pop stuff
    void add_pop(BasePop* pop);
    void sell_to_pops();
    void sell_type(int type);

    //Selling to brokers
    void sell_to_other_brokers();
    void distribute_from_order(const TradeOrder* order) override;
    void report_attempt(int type, int amount) override;

    // GDVIRTUAL1(_my_virtual_method_name, int); Ex

    // Process Hooks
    virtual void day_tick();
    virtual void month_tick();
};
