#include "peasant_pop.hpp"
#include <godot_cpp/core/class_db.hpp>

void PeasantPop::_bind_methods() {
}

void PeasantPop::initialize(int p_home_prov_id, Variant p_culture) {}

PeasantPop::PeasantPop(): RuralPop(-1, 0) {}

PeasantPop::PeasantPop(int p_home_prov_id, Variant p_culture): RuralPop(p_home_prov_id, p_culture) {}

PeasantPop::~PeasantPop() {}

int PeasantPop::get_people_per_pop() {
    return PEOPLE_PER_POP;
}