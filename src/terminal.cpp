#include "terminal.hpp"

#include <godot_cpp/core/class_db.hpp>

void Terminal::_bind_methods() {
    ClassDB::bind_method(D_METHOD("initialize", "location", "owner"), &Terminal::initialize);
    ClassDB::bind_method(D_METHOD("set_location", "location"), &Terminal::set_location);
    ClassDB::bind_method(D_METHOD("get_location"), &Terminal::get_location);
    ClassDB::bind_method(D_METHOD("get_player_id"), &Terminal::get_player_id);
    ClassDB::bind_method(D_METHOD("calculate_reward", "type", "amount"), &Terminal::calculate_reward);

    ClassDB::bind_static_method(Terminal::get_class_static(), D_METHOD("create", "location", "owner"), &Terminal::create);

    ClassDB::add_property(get_class_static(),  PropertyInfo(Variant::VECTOR2I, "location"), "set_location", "get_location");
    ClassDB::add_property(get_class_static(),  PropertyInfo(Variant::INT, "player_id"), "", "get_player_id");
}

void Terminal::set_location(const Vector2i p_location) {
    location = p_location;
}
Vector2i Terminal::get_location() const {
    return location;
}

int Terminal::get_player_id() const {
    return player_owner;
}

int Terminal::calculate_reward(int type, int amount) const {
    return type * amount;
}

Terminal* Terminal::create(const Vector2i p_location, const int p_owner) {
    return memnew(Terminal(p_location, p_owner));
}

void Terminal::initialize(const Vector2i p_location, int p_owner) {
    location = p_location;
    player_owner = p_owner;
}

String Terminal::_to_string() const {
    return String(get_location()) + ": " + itos(player_owner);
}

