#include "construction_component.hpp"

#include <godot_cpp/core/class_db.hpp>

void ConstructionComponent::add_construction_material(int type, int max_amount) {
    max_amounts[type] = max_amount;
}

godot::Dictionary ConstructionComponent::get_construction_materials_godot() const {
    godot::Dictionary d;
    for (const auto& [key, val]: materials) {
        d[key] = val;
    }
    return d;
}

godot::Dictionary ConstructionComponent::get_total_construction_materials_godot() const {
    godot::Dictionary d;
    for (const auto& [key, val]: max_amounts) {
        d[key] = val;
    }
    return d;
}

bool ConstructionComponent::is_needed_for_construction(int type) const {
    if (max_amounts.count(type)) {
        return max_amounts.at(type) == materials.at(type);
    }
    return false;
}

int ConstructionComponent::get_amount_of_type_needed_for_construction(int type) const {
    if (max_amounts.count(type)) {
        return max_amounts.at(type) - materials.at(type);
    }
    return 0;
}

bool ConstructionComponent::is_finished_constructing() const {
    if (materials.size() == 0) return false; // If not constructing than not finished to avoid upgrading
    for (const auto& [type, val]: materials) {
		if (max_amounts.count(type) && max_amounts.at(type) != val) {
            return false;
        }
    }
    return true;
}

bool ConstructionComponent::is_constructing() const {
    return !max_amounts.empty();
}

void ConstructionComponent::finish_construction() {
    max_amounts.clear();
    materials.clear();
}