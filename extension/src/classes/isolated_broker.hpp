#pragma once

#include "firm.hpp"
#include "base_pop.hpp"

using namespace godot;

/*
    Represents trading building that does not sit on a tile,
    only trades with the town in the same province, if multiple, then the closest.
    No local pricer and only adheres to town prices
    Creates cargo every month versus every day to increase efficiency

*/

class Town;
class Recipe;
class Province;

class IsolatedBroker : public Firm {
    GDCLASS(IsolatedBroker, Firm);

    protected:
    Vector2i local_town;
    static constexpr int MAX_STORAGE = 1000;
    static void _bind_methods();
    Recipe* recipe = nullptr;
    std::unordered_map<int, int> storage;
    void give_cargo_grain(Province* province, int pop_id);

    public:
    IsolatedBroker();
    IsolatedBroker(Vector2i p_location, int p_owner);

    std::unordered_map<int, float> get_outputs() const;
    std::unordered_map<int, float> get_inputs() const;
    float get_level() const;

    void add_pop(BasePop* pop);
    float get_wage() const;
    float get_theoretical_gross_profit() const;
    void pay_employees();

    void set_local_town(Vector2i p_town);
    Ref<Town> get_local_town() const;
    void sell_cargo();
    void sell_type(Ref<Town> town, int type, int amount);
    double get_batch_size() const;
    void create_recipe();
    virtual void month_tick();

    void consider_upgrade();
    void consider_degrade();
};