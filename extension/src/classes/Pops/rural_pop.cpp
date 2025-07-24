#include "rural_pop.hpp"
#include <godot_cpp/core/class_db.hpp>

void RuralPop::_bind_methods() {
     ClassDB::bind_static_method(get_class_static(), D_METHOD("create", "p_home_prov_id", "p_culture"), &RuralPop::create);
}

BasePop* RuralPop::create(int p_home_prov_id, Variant p_culture) {
    return memnew(RuralPop(p_home_prov_id, p_culture));
}

void RuralPop::initialize(int p_home_prov_id, Variant p_culture) {}

RuralPop::RuralPop(): BasePop(-1, 0) {}

RuralPop::RuralPop(int p_home_prov_id, Variant p_culture): BasePop(p_home_prov_id, p_culture) {}

RuralPop::~RuralPop() {}