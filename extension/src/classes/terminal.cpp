#include "terminal.hpp"

#include <godot_cpp/core/class_db.hpp>

void Terminal::_bind_methods() {
    ClassDB::bind_method(D_METHOD("set_location", "location"), &Terminal::set_location);
    ClassDB::bind_method(D_METHOD("get_location"), &Terminal::get_location);
    ClassDB::bind_method(D_METHOD("get_player_owner"), &Terminal::get_player_owner);

    ClassDB::bind_static_method(Terminal::get_class_static(), D_METHOD("create", "location", "owner"), &Terminal::create);
    ClassDB::bind_method(D_METHOD("initialize", "location", "owner"), &Terminal::initialize);

    ClassDB::add_property(Terminal::get_class_static(),  PropertyInfo(Variant::VECTOR2I, "location"), "set_location", "get_location");
    ClassDB::add_property(Terminal::get_class_static(),  PropertyInfo(Variant::INT, "player_owner"), "", "get_player_owner");
}

Terminal::Terminal(): terminal_id(-1) {
    location = Vector2i(0, 0);
    player_owner = 0;
}

Terminal::Terminal(const Vector2i p_location, const int p_owner): terminal_id(total_terminals++) {
    location = p_location;
    player_owner = p_owner;
}

void Terminal::set_location(const Vector2i p_location) {
    std::scoped_lock lock(m);
    location = p_location;
}

Vector2i Terminal::get_location() const {
    return location;
}

int Terminal::get_player_owner() const {
    std::scoped_lock lock(m);
    return player_owner;
}

int Terminal::get_terminal_id() const {
    return terminal_id;
}

Ref<Terminal> Terminal::create(const Vector2i p_location, const int p_owner) {
    // Static factory methods typically don't need locking unless accessing shared state.
    return Ref<Terminal>(memnew(Terminal(p_location, p_owner)));
}


void Terminal::initialize(const Vector2i p_location, int p_owner) {
    std::scoped_lock lock(m);
    location = p_location;
    player_owner = p_owner;
    print_error("DOES NOT CREATE TERMINAL ID");
}

std::atomic<int> Terminal::total_terminals = 0;