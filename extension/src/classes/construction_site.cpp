#include "../singletons/cargo_info.hpp"
#include "construction_site.hpp"

void ConstructionSite::_bind_methods() {
    ClassDB::bind_static_method(get_class_static(), D_METHOD("create", "new_location", "player_owner"), &ConstructionSite::create);
    ClassDB::bind_method(D_METHOD("initialize", "new_location", "player_owner"), &ConstructionSite::initialize);


    ClassDB::bind_method(D_METHOD("set_recipe", "recipe"), &ConstructionSite::set_recipe);
    ClassDB::bind_method(D_METHOD("get_recipe"), &ConstructionSite::get_recipe);
    ClassDB::bind_method(D_METHOD("has_recipe"), &ConstructionSite::has_recipe);
    ClassDB::bind_method(D_METHOD("destroy_recipe"), &ConstructionSite::destroy_recipe);
    ClassDB::bind_method(D_METHOD("get_construction_materials"), &ConstructionSite::get_construction_materials);
    
    ADD_SIGNAL(MethodInfo("Done_Building", PropertyInfo(Variant::OBJECT, "site")));
}

ConstructionSite::ConstructionSite(): FactoryTemplate() {

}

ConstructionSite::~ConstructionSite() {}

ConstructionSite::ConstructionSite(Vector2i new_location, int player_owner): FactoryTemplate(new_location, player_owner, {}, {}) {
    local_pricer = memnew(LocalPriceController);
}

Ref<ConstructionSite> ConstructionSite::create(Vector2i new_location, int player_owner) {
    return Ref<ConstructionSite>(memnew(ConstructionSite(new_location, player_owner)));
}

void ConstructionSite::initialize(Vector2i new_location, int player_owner) {
    FactoryTemplate::initialize(new_location, player_owner, {}, {});
}

//Recipe
void ConstructionSite::set_recipe(const Array recipe) {
    FactoryTemplate::initialize(get_location(), get_player_owner(), recipe[0], recipe[1]);
	create_construction_materials();
}

Array ConstructionSite::get_recipe() const {
    Dictionary in_d;
    m.lock();
    for (const auto& [key, val]: inputs) {
        in_d[key] = val;
    }
    Dictionary out_d;
    for (const auto& [key, val]: outputs) {
        out_d[key] = val;
    }
    m.unlock();
    Array a;
    a.push_back(in_d);
    a.push_back(out_d);
    return a;
}

bool ConstructionSite::has_recipe() const {
    std::scoped_lock lock(m);
    return inputs.size() != 0 || outputs.size() != 0;
}

void ConstructionSite::destroy_recipe() {
    std::scoped_lock lock(m);
    inputs.clear();
    outputs.clear();
}

//Materials
void ConstructionSite::create_construction_materials() {
    Ref<CargoInfo> cargo_info = CargoInfo::get_instance();
    create_construction_material(cargo_info->get_cargo_type("wood"), 1000);
}

void ConstructionSite::create_construction_material(int type, int amount) {
    add_accept(type);
    std::scoped_lock lock(m);
	max_amounts[type] = amount;
	construction_materials[type] = amount;
}

Dictionary ConstructionSite::get_construction_materials() const {
    Dictionary d;
    std::scoped_lock lock(m);
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

int ConstructionSite::get_max_storage() const {
    print_error("USE TYPE IN MAX STORAGE");
    return 0;
}

int ConstructionSite::get_max_storage(int type) const {
    std::scoped_lock lock(m);
    return max_amounts.at(type);
}

//Is buying
bool ConstructionSite::is_price_acceptable(int type, float pricePer) const {
    return get_local_price(type) * (MAX_TRADE_MARGIN) >= pricePer;
}
//To buy
int ConstructionSite::get_desired_cargo(int type, float pricePer) const {
    return get_desired_cargo_from_train(type, pricePer);
}

int ConstructionSite::get_desired_cargo_from_train(int type, float pricePer) const {
    if (does_accept(type)) {
        return std::min(get_max_storage(type) - get_cargo_amount(type), get_amount_can_buy(pricePer));
    }
    return 0;
}

void ConstructionSite::report_price(int type, float price) {
    std::scoped_lock lock(m);
    local_pricer -> move_price(type, price);
}

//Tickers
void ConstructionSite::day_tick() {}

void ConstructionSite::month_tick() {
    if (is_finished_constructing()) {
        emit_signal("Done_Building", this);
    }
}