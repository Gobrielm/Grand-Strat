
#include "money_controller.hpp"
#include <godot_cpp/core/class_db.hpp>

using namespace godot;

void MoneyController::_bind_methods() {
    ClassDB::bind_static_method(MoneyController::get_class_static(), D_METHOD("create", "peers"), &MoneyController::create);
    ClassDB::bind_static_method(MoneyController::get_class_static(), D_METHOD("get_instance"), &MoneyController::get_instance);

    ClassDB::bind_method(D_METHOD("add_peer", "new_id"), &MoneyController::add_peer);
    ClassDB::bind_method(D_METHOD("delete_peer", "id"), &MoneyController::delete_peer);
    ClassDB::bind_method(D_METHOD("add_money_to_player", "id", "amount"), &MoneyController::add_money_to_player);
    ClassDB::bind_method(D_METHOD("remove_money_from_player", "id", "amount"), &MoneyController::remove_money_from_player);
    ClassDB::bind_method(D_METHOD("get_money", "id"), &MoneyController::get_money);
    ClassDB::bind_method(D_METHOD("get_money_dictionary"), &MoneyController::get_money_dictionary);
    ClassDB::bind_method(D_METHOD("player_has_enough_money", "id", "amount"), &MoneyController::player_has_enough_money);
}

MoneyController::MoneyController() {
    money = std::unordered_map<int, float>();
}

void MoneyController::create(const Array& peers) {
    ERR_FAIL_COND_MSG(singleton_instance != nullptr, "Cannot create multiple instances of singleton!");
    MoneyController x;
    x.money[1] = INITIAL_AMOUNT_OF_MONEY; // Adds 1 for server

    for (int i = 0; i < peers.size(); i++) {
        int peer = peers[i];
        x.money[peer] = INITIAL_AMOUNT_OF_MONEY;
    }
}

MoneyController* MoneyController::get_instance() {
    ERR_FAIL_COND_V_MSG(singleton_instance == nullptr, nullptr, "Money_Manager has not been created but is being accessed");
    return singleton_instance;
}

void MoneyController::add_peer(int new_id) {
    ERR_FAIL_COND_MSG(money.count(new_id), "Peer already exists!");
    money[new_id] = INITIAL_AMOUNT_OF_MONEY;
}

void MoneyController::delete_peer(int id) {
    ERR_FAIL_COND_MSG(!money.count(id), "Peer doesn't exist!");
    money.erase(id);
}

void MoneyController::add_money_to_player(int id, float amount) {
    money[id] += amount;
}

void MoneyController::remove_money_from_player(int id, float amount) {
    add_money_to_player(id, -amount);
}

float MoneyController::get_money(int id) const {
    return money.at(id);
}

Dictionary MoneyController::get_money_dictionary() const {
    Dictionary d;
    for (const auto &p: money) {
        d[p.first] = p.second;
    }
    return d;
}

bool MoneyController::player_has_enough_money(int id, int amount) const {
    return get_money(id) >= amount;
}


MoneyController* MoneyController::singleton_instance = nullptr;
const int MoneyController::INITIAL_AMOUNT_OF_MONEY = 100000;