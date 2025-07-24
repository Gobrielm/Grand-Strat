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

    void initialize(int p_home_prov_id = -1, Variant p_culture = 0);

    PeasantPop();
    PeasantPop(int p_home_prov_id, Variant p_culture);
    virtual ~PeasantPop();
};