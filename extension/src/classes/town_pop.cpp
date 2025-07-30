#include "town_pop.hpp"
#include <godot_cpp/core/class_db.hpp>

void TownPop::_bind_methods() {
}

TownPop::TownPop::TownPop(): BasePop(-1, Vector2i(0, 0), 0) {}

TownPop::TownPop(int p_home_prov_id, Vector2i p_location, Variant p_culture): BasePop(p_home_prov_id, p_location, p_culture) {}

TownPop::~TownPop() {}

int TownPop::get_people_per_pop() {
    return PEOPLE_PER_POP;
}