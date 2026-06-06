#pragma once

#include <memory>

#include "classes/base_pop.hpp"
#include "classes/trade_order.hpp"
#include "classes/components/capital_component.hpp"
#include "classes/components/owner_component.hpp"
#include "classes/components/storage_component.hpp"
#include "classes/components/position_component.hpp"
#include "classes/components/employer_component.hpp"

#include <godot_cpp/classes/object.hpp>

using namespace godot;

/*
    Represents trading building that does not sit on a tile,
    only trades with the town in the same province, if multiple, then the closest.
    No local pricer and only adheres to town prices
    Creates cargo every month versus every day to increase efficiency

*/

class Town;
class Recipe;

class SubsistenceFarm {

    std::unordered_map<int, std::shared_ptr<TradeOrder>> orders; // Active orders sent to towns to buy/sell

    public:

    PositionComponent position;
    StorageComponent storage;
    OwnerComponent owner;
    CapitalComponent capital;
    EmployerComponent employer;

    StorageComponent last_month_storage;
    // type -> true if bought/sold more than/equal to needed, false if sold/bought less than neccessary
    std::vector<int> storage_delta_indicator;

    SubsistenceFarm();
    SubsistenceFarm(Vector2i p_location, int p_owner);
    SubsistenceFarm(const SubsistenceFarm& other);

    SubsistenceFarm& operator=(const SubsistenceFarm&);

    void add_pop(Town& town, BasePop* pop);
    float get_wage();

    double get_batch_size();
    void create_recipe();
    void month_tick();

    static EmployerComponent get_default_employer_component();

    void consider_upgrade();
    void consider_degrade();

    void adjust_trade_orders(Town& town);

    float get_current_price(int type) const;
    int get_supply(int type) const;
    int get_demand(int type) const;
};