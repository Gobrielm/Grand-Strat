#pragma once

#include <classes/base_pop.hpp>
#include <classes/components/position_component.hpp>
#include <classes/components/storage_component.hpp>
#include <classes/components/owner_component.hpp>

#include <godot_cpp/classes/ref_counted.hpp>

using namespace godot;

/*
    Represents trading building that does not sit on a tile,
    only trades with the town in the same province, if multiple, then the closest.
    No local pricer and only adheres to town prices
    Creates cargo every month versus every day to increase efficiency

*/

class Town;
class Recipe;

class SubsistenceFarm : public RefCounted {
    GDCLASS(SubsistenceFarm, RefCounted);

    PositionComponent town_pos;
    
    void give_cargo_grain(int pop_id);

    protected:
    
    static void _bind_methods();
    

    public:

    PositionComponent position;
    StorageComponent storage;
    OwnerComponent owner;
    Recipe recipe;

    SubsistenceFarm();
    SubsistenceFarm(Vector2i p_location, int p_owner);

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
    void month_tick();

    void consider_upgrade();
    void consider_degrade();
};