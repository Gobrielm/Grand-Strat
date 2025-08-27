#pragma once

#include "factory.hpp"

using namespace godot;

class AiFactory : public Factory {
    GDCLASS(AiFactory, Factory)
    static constexpr int CASH_NEEDED = 3000;


protected:
    static void _bind_methods();

public:
    AiFactory();
    virtual ~AiFactory();
    AiFactory(Vector2i new_location, int player_owner, Recipe* p_recipe);

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
    virtual void day_tick() override;
    virtual void month_tick() override;
};
