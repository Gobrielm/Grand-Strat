#pragma once

#include"base_pop.hpp"
#include <unordered_map>

using namespace godot;

class TownPop : public BasePop {
    GDCLASS(TownPop, BasePop);

    static constexpr int PEOPLE_PER_POP = 1000;
    static constexpr int INITIAL_WEALTH = 1000;

    protected:
    static void _bind_methods();

    public:

    TownPop();
    TownPop(int p_home_prov_id, Vector2i p_location, Variant p_culture);
    virtual ~TownPop();

    static int get_people_per_pop();
};