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

Ref<Terminal> Hold::create(const Vector2i new_location, const int player_owner, const int p_max_amount) {
    return Ref<Terminal>(memnew(Hold(new_location, player_owner, p_max_amount)));
}   

Hold::Hold(): Firm() {
    ERR_FAIL_COND_MSG(NUMBER_OF_GOODS == 0, "NUMBER_OF_GOODS was not set");

    for (int i = 0; i < NUMBER_OF_GOODS; i++) {
        storage[i] = 0;
    }
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

float Hold::add_cargo(int type, float amount) {
    float amount_to_add = get_amount_to_add(type, amount);
    std::scoped_lock lock(m);
    storage[type] += amount_to_add;
    return amount_to_add;
}

float Hold::get_cargo_amount(int type) const {
    std::scoped_lock lock(m);
    ERR_FAIL_COND_V_EDMSG(!storage.count(type), 0, "No cargo of type: " + String::num(type));
    return storage.at(type);
}

int Hold::get_cargo_amount_outside(int type) const {
    std::scoped_lock lock(m);
    if (storage.count(type) == 0) {
        ERR_PRINT("Storage has no such type: " + String::num_int64(type));
        return 0; // or return some error value like -1, depending on your logic
    }
    return storage.at(type);
}

void Hold::remove_cargo(int type, float amount) {
    std::scoped_lock lock(m);
    storage[type] -= amount;
    ERR_FAIL_COND_MSG(storage[type] < 0, "Storage went below zero! It is " + String::num(storage[type]));
}

float Hold::transfer_cargo(int type, float amount) {
    float val = std::min(amount, get_cargo_amount(type));
    remove_cargo(type, val);
    return val;
}

int Hold::transfer_cargo(int type, int amount) {
    int val = std::min(amount, int(get_cargo_amount(type)));
    remove_cargo(type, val);
    return val;
}

float Hold::get_amount_to_add(int type, float amount) const {
    return std::min(get_max_storage() - get_current_hold_total(), amount);
}

Dictionary Hold::get_current_hold() const {
    std::scoped_lock lock(m);
    Dictionary d;
    for (const auto &pair : storage) {
        d[pair.first] = (round(pair.second * 100)) / 100.0;
    }
    return d;
}

void Hold::set_current_hold(Dictionary hold) {
    std::scoped_lock lock(m);
    Array keys = hold.keys();
    for (int i = 0; i < keys.size(); i++) {
        int type = keys[i];
        storage[type] = hold[type];
    }
}

float Hold::get_current_hold_total() const {
    std::scoped_lock lock(m);
    float total = 0;
    for (const auto &pair : storage) {
        total += pair.second;
    }
    return round(total);
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
    std::scoped_lock lock(m);
    max_amount += p_amount;
}

void Hold::set_max_storage(int p_amount) {
    std::scoped_lock lock(m);
    max_amount = p_amount;
}

bool Hold::does_accept(int type) const {
    return get_current_hold_total() < get_max_storage();
}

void Hold::set_number_of_goods(int p_num) {
    NUMBER_OF_GOODS = p_num;
}

int Hold::NUMBER_OF_GOODS = 52;
const int Hold::DEFAULT_MAX_STORAGE = 50000;