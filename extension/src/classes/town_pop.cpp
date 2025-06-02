#include "town_pop.hpp"
#include <godot_cpp/core/class_db.hpp>

void TownPop::_bind_methods() {
     ClassDB::bind_static_method(get_class_static(), D_METHOD("create", "p_home_prov_id", "p_culture"), &TownPop::create);
}

BasePop* TownPop::create(int p_home_prov_id, Variant p_culture) {
    return memnew(TownPop(p_home_prov_id, p_culture));
}

void TownPop::initialize(int p_home_prov_id, Variant p_culture) {}

TownPop::TownPop::TownPop(): BasePop(-1, 0) {}

TownPop::TownPop(int p_home_prov_id, Variant p_culture): BasePop(p_home_prov_id, p_culture) {}

TownPop::~TownPop() {}