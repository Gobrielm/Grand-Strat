#include "hold.hpp"
#include <godot_cpp/core/class_db.hpp>

using namespace godot;

void Hold::_bind_methods() {
    ClassDB::bind_static_method(get_class_static(), D_METHOD("create", "new_location", "player_owner", "p_max_amount"), &Hold::create);
    ClassDB::bind_static_method(get_class_static(), D_METHOD("set_num_of_goods", "p_num"), &Hold::set_number_of_goods);

    ClassDB::bind_method(D_METHOD("initialize", "new_location", "player_owner", "max_amount"), &Hold::initialize);
    ClassDB::bind_method(D_METHOD("add_cargo", "type", "amount"), &Hold::add_cargo);
    ClassDB::bind_method(D_METHOD("get_cargo_amount", "type"), &Hold::get_cargo_amount);
    ClassDB::bind_method(D_METHOD("remove_cargo", "type", "amount"), &Hold::remove_cargo);
    ClassDB::bind_method(D_METHOD("transfer_cargo", "type", "amount"), &Hold::transfer_cargo);
    ClassDB::bind_method(D_METHOD("get_amount_to_add", "type", "amount"), &Hold::get_amount_to_add);
    ClassDB::bind_method(D_METHOD("get_current_hold"), &Hold::get_current_hold);
    ClassDB::bind_method(D_METHOD("set_current_hold", "hold"), &Hold::set_current_hold);
    ClassDB::bind_method(D_METHOD("get_current_hold_total"), &Hold::get_current_hold_total);
    ClassDB::bind_method(D_METHOD("is_full"), &Hold::is_full);
    ClassDB::bind_method(D_METHOD("is_empty"), &Hold::is_empty);
    ClassDB::bind_method(D_METHOD("get_max_storage"), &Hold::get_max_storage);
    ClassDB::bind_method(D_METHOD("change_max_storage", "amount"), &Hold::change_max_storage);
    ClassDB::bind_method(D_METHOD("set_max_storage", "amount"), &Hold::set_max_storage);
    ClassDB::bind_method(D_METHOD("does_accept", "type"), &Hold::does_accept);
}

Terminal* Hold::create(const Vector2i new_location, const int player_owner, const int p_max_amount) {
    return memnew(Hold(new_location, player_owner, p_max_amount));
}   

Hold::Hold(const Vector2i new_location, const int player_owner, const int p_max_amount): max_amount(p_max_amount), Firm(new_location, player_owner) {
    ERR_FAIL_COND_MSG(NUMBER_OF_GOODS == 0, "NUMBER_OF_GOODS was not set");

    for (int i = 0; i < NUMBER_OF_GOODS; i++) {
        storage[i] = 0;
    }
}

void Hold::initialize(Vector2i new_location, int player_owner, int p_max_amount) {
    ERR_FAIL_COND_MSG(NUMBER_OF_GOODS == 0, "NUMBER_OF_GOODS was not set");
    
    Firm::initialize(new_location, player_owner);
    max_amount = p_max_amount;
    
    for (int i = 0; i < NUMBER_OF_GOODS; i++) {
        storage[i] = 0;
    }
}

int Hold::add_cargo(int type, int amount) {
    int amount_to_add = get_amount_to_add(type, amount);
    storage[type] += amount_to_add;
    return amount_to_add;
}

int Hold::get_cargo_amount(int type) const {
    return storage.at(type);
}

void Hold::remove_cargo(int type, int amount) {
    storage[type] -= amount;
    ERR_FAIL_COND_MSG(storage[type] < 0, "Storage went below zero!");
}

int Hold::transfer_cargo(int type, int amount) {
    int val = std::min(amount, get_cargo_amount(type));
    remove_cargo(type, val);
    return val;
}

int Hold::get_amount_to_add(int type, int amount) const {
    return std::min(max_amount - get_current_hold_total(), amount);
}

Dictionary Hold::get_current_hold() const {
    Dictionary d;
    for (const auto &pair : storage) {
        d[pair.first] = pair.second;
    }
    return d;
}

void Hold::set_current_hold(Dictionary hold) {
    Array keys = hold.keys();
    for (int i = 0; i < keys.size(); i++) {
        int type = keys[i];
        storage[type] = hold[type];
    }
}

int Hold::get_current_hold_total() const {
    int total = 0;
    for (const auto &pair : storage) {
        total += pair.second;
    }
    return total;
}

bool Hold::is_full() const {
    return get_current_hold_total() >= max_amount;
}

bool Hold::is_empty() const {
    return get_current_hold_total() == 0;
}

int Hold::get_max_storage() const {
    return max_amount;
}

void Hold::change_max_storage(int p_amount) {
    max_amount += p_amount;
}

void Hold::set_max_storage(int p_amount) {
    max_amount = p_amount;
}

bool Hold::does_accept(int type) const {
    return get_current_hold_total() < max_amount;
}

void Hold::set_number_of_goods(int p_num) {
    NUMBER_OF_GOODS = p_num;
}

int Hold::NUMBER_OF_GOODS = 52;
const int Hold::DEFAULT_MAX_STORAGE = 50;