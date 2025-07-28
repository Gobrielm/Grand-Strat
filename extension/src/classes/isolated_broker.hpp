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
    
    Vector2i local_town;
    Recipe* recipe = nullptr;

    protected:
    static constexpr int MAX_STORAGE = 1000;
    static void _bind_methods();
    std::unordered_map<int, int> storage;

    public:
    IsolatedBroker();

    void sell_cargo();
    void sell_type(Ref<Town> town, int type, int amount);
    bool check_inputs() const;
    bool check_outputs() const;
    void create_recipe();
    void day_tick();
    void month_tick();
};