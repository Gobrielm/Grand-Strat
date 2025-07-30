#include "peasant_pop.hpp"
#include <godot_cpp/core/class_db.hpp>

void PeasantPop::_bind_methods() {
}

PeasantPop::PeasantPop(): RuralPop(-1, Vector2i(0, 0), 0) {}

PeasantPop::PeasantPop(int p_home_prov_id, Vector2i p_location, Variant p_culture): RuralPop(p_home_prov_id, p_location, p_culture) {}

PeasantPop::~PeasantPop() {}

int PeasantPop::get_people_per_pop() {
    return PEOPLE_PER_POP;
}