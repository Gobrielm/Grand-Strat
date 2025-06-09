#pragma once

#include "fixed_hold.hpp"
#include "trade_order.hpp"
#include "terminal.hpp"
#include "local_price_controller.hpp"
#include "../utility/vector2i_hash.hpp"
#include "../singletons/terminal_map.hpp"
#include <unordered_map>
#include <unordered_set>
#include <memory>

class Broker : public FixedHold {
    GDCLASS(Broker, FixedHold);
    std::unordered_map<int, TradeOrder*> trade_orders;
    float change_in_cash = 0.0f;


    protected:
    static void _bind_methods();

    public:
    std::unordered_set<Vector2i, godot_helpers::Vector2iHasher> connected_brokers;
    LocalPriceController* local_pricer = nullptr;

    static Ref<Terminal> create(const Vector2i new_location, const int player_owner, const int p_max_amount = DEFAULT_MAX_STORAGE);

    Broker();
    Broker(const Vector2i new_location, const int player_owner, const int p_max_amount = DEFAULT_MAX_STORAGE);
    virtual ~Broker();
    virtual void initialize(const Vector2i new_location, const int player_owner, const int p_max_amount = DEFAULT_MAX_STORAGE);

    bool can_afford(int price) const;
    Dictionary get_local_prices() const;
    virtual float get_local_price(int type) const;

    virtual int get_desired_cargo(int type, float pricePer) const;
    int get_desired_cargo_from_train(int type) const;

    virtual bool is_price_acceptable(int type, float pricePer) const;

    void buy_cargo(int type, int amount, float price);
    int sell_cargo(int type, int amount, float price);
    int add_cargo(int type, int amount) override;
    int add_cargo_ignore_accepts(int type, int amount) override;

    virtual void place_order(int type, int amount, bool buy, float maxPrice);
    virtual void edit_order(int type, int amount, bool buy, float maxPrice);

    TradeOrder* get_order(int type) const;
    std::unordered_map<int, TradeOrder*> get_orders();
    Dictionary get_orders_dict();
    virtual void remove_order(int type);

    virtual void add_connected_broker(Ref<Broker> broker);
    virtual void remove_connected_broker(const Ref<Broker> broker);
    Dictionary get_connected_broker_locations();

    virtual void distribute_cargo(); // abstract
    virtual void distribute_from_order(const TradeOrder* order);
    void distribute_to_order(Ref<Broker> otherBroker, const TradeOrder* order);

    void report_attempt_to_sell(int type, int amount);

    
};
