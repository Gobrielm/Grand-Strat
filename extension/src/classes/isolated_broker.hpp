#pragma once

#include "firm.hpp"
#include "base_pop.hpp"

using namespace godot;

/*
    Represents trading building that does not sit on a tile,
    only trades with the town in the same province, if multiple, then the closest.
    No local pricer and only adheres to town prices


*/

class Town;
class Recipe;

class IsolatedBroker : public Firm {
    GDCLASS(IsolatedBroker, Firm);

    protected:
    Vector2i local_town;
    static constexpr int MAX_STORAGE = 1000;
    static void _bind_methods();
    Recipe* recipe = nullptr;
    std::unordered_map<int, int> storage;

    public:
    IsolatedBroker(int p_owner = 0);

    void add_pop(BasePop* pop);

    void sell_cargo();
    void sell_type(Ref<Town> town, int type, int amount);
    bool check_inputs() const;
    bool check_outputs() const;
    void create_recipe();
    void day_tick();
    void month_tick();

    void consider_upgrade();
    void consider_degrade();
};