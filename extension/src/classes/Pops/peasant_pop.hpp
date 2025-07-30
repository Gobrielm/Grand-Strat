#pragma once

#include"rural_pop.hpp"
#include <unordered_map>

using namespace godot;

class PeasantPop : public RuralPop {
    GDCLASS(PeasantPop, RuralPop);

    static constexpr int PEOPLE_PER_POP = 1000;
    static constexpr int INITIAL_WEALTH = 100;

    protected:
    static void _bind_methods();

    public:

    PeasantPop();
    PeasantPop(int p_home_prov_id, Vector2i p_location, Variant p_culture);
    virtual ~PeasantPop();

    static int get_people_per_pop();
};