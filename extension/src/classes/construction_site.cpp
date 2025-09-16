#include "construction_site.hpp"
#include "../singletons/cargo_info.hpp"
#include "factory_utility/recipe.hpp"
#include <godot_cpp/core/class_db.hpp>
#include <mutex>

void ConstructionSite::_bind_methods() {
    ClassDB::bind_static_method(get_class_static(), D_METHOD("create", "new_location", "player_owner"), &ConstructionSite::create);
    ClassDB::bind_method(D_METHOD("initialize", "new_location", "player_owner"), &ConstructionSite::initialize);

    ClassDB::bind_method(D_METHOD("has_recipe"), &ConstructionSite::has_recipe);
    ClassDB::bind_method(D_METHOD("destroy_recipe"), &ConstructionSite::destroy_recipe);
    
    ADD_SIGNAL(MethodInfo("Done_Building", PropertyInfo(Variant::OBJECT, "site")));
}

ConstructionSite::ConstructionSite(): FactoryTemplate() {
}

ConstructionSite::~ConstructionSite() {}

ConstructionSite::ConstructionSite(Vector2i new_location, int player_owner): FactoryTemplate(new_location, player_owner, memnew(Recipe)) {
    local_pricer = new(LocalPriceController);
}

Ref<ConstructionSite> ConstructionSite::create(Vector2i new_location, int player_owner) {
    return Ref<ConstructionSite>(memnew(ConstructionSite(new_location, player_owner)));
}

void ConstructionSite::initialize(Vector2i new_location, int player_owner) {
    FactoryTemplate::initialize(new_location, player_owner, memnew(Recipe));
}

//Recipe
void ConstructionSite::set_recipe(Recipe* p_recipe) {
    std::scoped_lock lock(m);
    recipe = p_recipe;
	create_construction_materials_unsafe();
}

Array ConstructionSite::get_recipe() const {    
    Array a;
    if (recipe == nullptr) return Array();
    {
        std::scoped_lock lock(m);
        Dictionary d1;
        Dictionary d2;
        for (const auto& [type, amount]: recipe->get_inputs()) {
            d1[type] = round(amount * 100) / 100;
        }   
        for (const auto& [type, amount]: recipe->get_outputs()) {
            d2[type] = round(amount * 100) / 100;
        }  
        
        a.push_back(d1);
        a.push_back(d2);
    }
    return a;
}

bool ConstructionSite::has_recipe() const {
    std::scoped_lock lock(m);
    return recipe->has_recipe();
}

void ConstructionSite::destroy_recipe() {
    std::scoped_lock lock(m);
    delete (recipe);
    recipe = nullptr;
}

//Materials

float ConstructionSite::get_max_storage() const {
    print_error("USE TYPE IN MAX STORAGE");
    return 0;
}

float ConstructionSite::get_max_storage(int type) const {
    std::scoped_lock lock(m);
    return get_max_storage_unsafe(type);
}

float ConstructionSite::get_max_storage_unsafe(int type) const {
    std::scoped_lock lock(m);
    return max_amounts_of_construction_materials.count(type) ? max_amounts_of_construction_materials.at(type): 0;
}

//Is buying
bool ConstructionSite::is_price_acceptable(int type, float pricePer) const {
    return true; // Always buy as construction site
}

int ConstructionSite::get_desired_cargo_from_train(int type, float pricePer) const {
    if (does_accept(type)) {
        return std::min(int(get_max_storage(type) - get_cargo_amount(type)), get_amount_can_buy(pricePer));
    }
    return 0;
}

//Tickers
void ConstructionSite::day_tick() {}

void ConstructionSite::month_tick() {
    if (is_finished_constructing()) {
        emit_signal("Done_Building", this);
    }
}