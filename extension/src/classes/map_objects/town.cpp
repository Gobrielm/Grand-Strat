#include "town.hpp"
#include <src/singletons/cargo_info.hpp>
#include <godot_cpp/core/class_db.hpp>

using namespace godot;

void Town::_bind_methods() {
    
}

Town::Town(std::pair<int, int> p_position): position(p_position, BuildingType::TOWN) {}

Town::Town(std::pair<int, int> p_position): position(p_position, BuildingType::TOWN) {}

Town::Town(Town& town): position(town.position), owner(town.owner), mp(town.mp) {}

Town Town::operator=(Town &town) {
    return Town(town);
}

void Town::add_factory(int factory_owner, int factory_id) {
    internal_factories[factory_owner].push_back(factory_id);
}

void Town::add_company(int company_id) {
    internal_companies.insert(company_id);
}

std::vector<int> Town::get_factory_ids() const {
    std::vector<int> v;
    for (const auto &[__, owner_list]: internal_factories){
        for (int id: owner_list) {
            v.push_back(id);
        }
    }   
    return v;
}

//Pop stuff
void Town::add_pop(int pop_id) {
    ERR_FAIL_COND_MSG(town_pop_ids.count(pop_id), "Pop of id has already been created");
    town_pop_ids.insert(pop_id);
}

int Town::get_total_pops() const {
    return town_pop_ids.size();
}

