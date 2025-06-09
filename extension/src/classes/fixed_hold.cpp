#include "fixed_hold.hpp"
#include <godot_cpp/core/class_db.hpp>

using namespace godot;

void FixedHold::_bind_methods() {
    ClassDB::bind_static_method(get_class_static(), D_METHOD("create", "new_location", "player_owner", "p_max_amount"), &FixedHold::create);
    
    ClassDB::bind_method(D_METHOD("initialize", "new_location", "player_owner", "p_max_amount"), &FixedHold::initialize);
    ClassDB::bind_method(D_METHOD("add_cargo", "type", "amount"), &FixedHold::add_cargo);
    ClassDB::bind_method(D_METHOD("add_cargo_ignore_accepts", "type", "amount"), &FixedHold::add_cargo_ignore_accepts);
    ClassDB::bind_method(D_METHOD("transfer_cargo", "type", "amount"), &FixedHold::transfer_cargo);
    ClassDB::bind_method(D_METHOD("get_desired_cargo", "type"), &FixedHold::get_desired_cargo);
    ClassDB::bind_method(D_METHOD("reset_accepts"), &FixedHold::reset_accepts);
    ClassDB::bind_method(D_METHOD("get_accepts"), &FixedHold::get_accepts);
    ClassDB::bind_method(D_METHOD("add_accept", "type"), &FixedHold::add_accept);
    ClassDB::bind_method(D_METHOD("remove_accept", "type"), &FixedHold::remove_accept);
    ClassDB::bind_method(D_METHOD("does_accept", "type"), &FixedHold::does_accept);
}

Ref<FixedHold> FixedHold::create(const Vector2i new_location, const int player_owner, const int p_max_amount) {
    return Ref<FixedHold>(memnew(FixedHold(new_location, player_owner, p_max_amount)));
}

FixedHold::FixedHold(): Hold() {}

FixedHold::FixedHold(const Vector2i new_location, const int player_owner, const int p_max_amount): Hold(new_location, player_owner, p_max_amount) {
    accepts = std::unordered_set<int>();
}


void FixedHold::initialize(Vector2i new_location, int player_owner, const int p_max_amount) {
    Hold::initialize(new_location, player_owner, p_max_amount);
    accepts = std::unordered_set<int>();
}

int FixedHold::add_cargo(int type, int amount) {
    if (does_accept(type)) {
        return Hold::add_cargo(type, amount);
    }
    return 0;
}

int FixedHold::add_cargo_ignore_accepts(int type, int amount) {
    return Hold::add_cargo(type, amount);
}

int FixedHold::transfer_cargo(int type, int amount) {
    int new_amount = std::min(get_cargo_amount(type), amount);
    remove_cargo(type, new_amount);
    return new_amount;
}

int FixedHold::get_desired_cargo(int type) const {
    if (does_accept(type)) {
        return get_max_storage() - get_cargo_amount(type);
    }
    return 0;
}

void FixedHold::reset_accepts() {
    accepts.clear();
}

Dictionary FixedHold::get_accepts() const {
    Dictionary dict;
    for (const int& type : accepts) {
        dict[type] = true;
    }
    return dict;
}

std::vector<bool> FixedHold::get_accepts_vector() const {
    std::vector<bool> v;
    v.resize(accepts.size());
    for (const int& type: accepts) {
        v[type] = true;
    }
    return v;
}

void FixedHold::add_accept(int type) {
    accepts.insert(type);
}

void FixedHold::remove_accept(int type) {
    accepts.erase(type);
}

bool FixedHold::does_accept(int type) const {
    return accepts.count(type);
}

void FixedHold::clear_accepts() {
    accepts.clear();
}