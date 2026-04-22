#include "town.hpp"
#include "singletons/cargo_info.hpp"
#include <godot_cpp/core/class_db.hpp>

using namespace godot;

Town::Town(std::pair<int, int> p_position): position(p_position, BuildingType::TOWN) {}

Town::Town(const Town& other): position(other.position), owner(other.owner), mp(other.mp) {}

Town& Town::operator=(const Town &other) {
    if (this == &other) return *this;

    mp = other.mp;
    position = other.position;
    owner = other.owner;
    
    internal_companies = other.internal_companies;
    internal_factories = other.internal_factories;
    
    return *this;
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

void Town::add_pop(int pop_id) {

}

int Town::get_total_pops() const {
    return 0;
}