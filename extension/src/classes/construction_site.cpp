
#include "construction_site.hpp"

void ConstructionSite::_bind_methods() {
    ClassDB::bind_static_method(get_class_static(), D_METHOD("create", "new_location", "player_owner"), &ConstructionSite::create);
    ClassDB::bind_method(D_METHOD("initialize", "new_location", "player_owner"), &ConstructionSite::initialize);


    ClassDB::bind_method(D_METHOD("set_recipe", "recipe"), &ConstructionSite::set_recipe);
    ClassDB::bind_method(D_METHOD("get_recipe"), &ConstructionSite::get_recipe);
    ClassDB::bind_method(D_METHOD("has_recipe"), &ConstructionSite::has_recipe);
    ClassDB::bind_method(D_METHOD("get_construction_materials"), &ConstructionSite::get_construction_materials);
    ClassDB::bind_method(D_METHOD("create_construction_materials", "dictionary"), &ConstructionSite::create_construction_materials);
    

    ADD_SIGNAL(MethodInfo("Request_construction_materials", PropertyInfo(Variant::OBJECT, "site")));
    ADD_SIGNAL(MethodInfo("Done_Building", PropertyInfo(Variant::OBJECT, "site")));
}

ConstructionSite::ConstructionSite(): FactoryTemplate() {

}

ConstructionSite::~ConstructionSite() {}

ConstructionSite::ConstructionSite(Vector2i new_location, int player_owner): FactoryTemplate(new_location, player_owner, {}, {}) {

}

Terminal* ConstructionSite::create(Vector2i new_location, int player_owner) {
    return memnew(ConstructionSite(new_location, player_owner));
}

void ConstructionSite::initialize(Vector2i new_location, int player_owner) {
    FactoryTemplate::initialize(new_location, player_owner, {}, {});
}

//Recipe
void ConstructionSite::set_recipe(const Array recipe) {
    FactoryTemplate::initialize(get_location(), get_player_owner(), recipe[0], recipe[1]);
    emit_signal("Request_construction_materials", this);
	//create_construction_materials();
}

void ConstructionSite::destroy_recipe() {
    inputs = {};
	outputs = {};
}

Array ConstructionSite::get_recipe() const {
    Dictionary in_d;
    for (const auto& [key, val]: inputs) {
        in_d[key] = val;
    }
    Dictionary out_d;
    for (const auto& [key, val]: outputs) {
        out_d[key] = val;
    }
    Array a;
    a.push_back(in_d);
    a.push_back(out_d);
    return a;
}

bool ConstructionSite::has_recipe() const {
    return inputs.size() != 0 && outputs.size() != 0;
}

//Materials
void ConstructionSite::create_construction_materials(const Dictionary d) {
    Array keys = d.keys();
    for (int i = 0; i < keys.size(); i++) {
        int type = keys[i];
        create_construction_material(type, d[type]);
    }
}

void ConstructionSite::create_construction_material(int type, int amount) {
    add_accept(type);
	max_amounts[type] = amount;
	construction_materials[type] = 50;
}

Dictionary ConstructionSite::get_construction_materials() const {
    Dictionary d;
    for (const auto& [key, val]: construction_materials) {
        d[key] = val;
    }
    return d;
}

bool ConstructionSite::is_finished_constructing() const {
    for (const auto& [type, val]: construction_materials) {
		if (get_cargo_amount(type) < val) {
            return false;
        }
    }
    return true;
}

void ConstructionSite::month_tick() {
    if (is_finished_constructing()) {
        emit_signal("Done_Building", this);
    }
}