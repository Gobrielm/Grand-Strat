#pragma once

#include"base_pop.hpp"
#include <unordered_map>

using namespace godot;

class RuralPop : public BasePop {
    GDCLASS(RuralPop, BasePop);

    static constexpr int PEOPLE_PER_POP = 1000;
    static constexpr int INITIAL_WEALTH = 100;

    protected:
    static void _bind_methods();

    public:

    static BasePop* create(int p_home_prov_id = -1, Variant p_culture = 0);
    void initialize(int p_home_prov_id = -1, Variant p_culture = 0);

    RuralPop();
    RuralPop(int p_home_prov_id, Variant p_culture);
    virtual ~RuralPop();
};