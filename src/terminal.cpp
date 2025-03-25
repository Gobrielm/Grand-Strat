#include "terminal.hpp"

#include <godot_cpp/core/class_db.hpp>

void Terminal::_bind_methods() {
    ClassDB::bind_method(D_METHOD("set_location", "location"), &Terminal::set_location);
    ClassDB::bind_method(D_METHOD("get_location"), &Terminal::get_location);
    ClassDB::bind_method(D_METHOD("get_player_id"), &Terminal::get_player_id);
    ClassDB::bind_method(D_METHOD("calculate_reward"), &Terminal::calculate_reward);
    
    ClassDB::bind_static_method(Terminal::get_class_static(), D_METHOD("create", "location", "owner"), &Terminal::create);

}

void Terminal::set_location(Vector2i p_location) {
    location = p_location;
}
Vector2i Terminal::get_location() const {
    return location;
}

int Terminal::get_player_id() const {
    return player_owner;
}

int Terminal::calculate_reward() const {
    return 0;
}

Terminal* Terminal::create(const Vector2i p_location, const int p_owner) {
    return memnew(Terminal(p_location, p_owner));
}

String Terminal::_to_string() const {
    return String(get_location()) + ": " + itos(player_owner);
}