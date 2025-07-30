#include "rural_pop.hpp"
#include <godot_cpp/core/class_db.hpp>

void RuralPop::_bind_methods() {
}

RuralPop::RuralPop(): BasePop() {}

RuralPop::RuralPop(int p_home_prov_id, Vector2i p_location, Variant p_culture): BasePop(p_home_prov_id, p_location, p_culture) {}

RuralPop::~RuralPop() {}

int RuralPop::get_people_per_pop() {
    return PEOPLE_PER_POP;
}