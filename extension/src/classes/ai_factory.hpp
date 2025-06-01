#pragma once

#include "factory.hpp"

using namespace godot;

class AiFactory : public Factory {
    GDCLASS(AiFactory, Factory)
    static constexpr int CASH_NEEDED_MULTIPLIER = 5;
    static constexpr float MAX_AMOUNT_WANTED = 0.75;


protected:
    static void _bind_methods();

public:
    AiFactory();
    ~AiFactory();
    AiFactory(Vector2i new_location, int player_owner, Dictionary new_inputs, Dictionary new_outputs);

    static Terminal* create(Vector2i new_location, int player_owner, Dictionary new_inputs, Dictionary new_outputs);

    virtual void initialize(Vector2i new_location, int player_owner, Dictionary new_inputs, Dictionary new_outputs);

    //Orders
    void change_orders();
    void change_buy_orders();
    void change_sell_orders();
    void change_order(int type, bool buy);
    
    //Upgrades
    void consider_upgrade();
    void consider_upgrade_primary();
    void consider_upgrade_secondary();


    // Process Hooks
    virtual void day_tick();
    virtual void month_tick();
};
