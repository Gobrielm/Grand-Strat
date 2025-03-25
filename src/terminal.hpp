#pragma once

#include <godot_cpp/classes/object.hpp>

using namespace godot;

class Terminal : public Object {
    GDCLASS(Terminal, Object);

    Vector2i location;
    int player_owner;

    protected:
    static void _bind_methods();
    String _to_string() const;

    public:
    void set_location(Vector2i p_location);
    Vector2i get_location() const;
    int get_player_id() const;
    int calculate_reward() const;
    static Terminal* create(const Vector2i p_location, const int p_owner);
    
    
    _FORCE_INLINE_ Terminal() {}
    _FORCE_INLINE_ Terminal(const Vector2i p_location, const int p_owner) {
        location = p_location;
        player_owner = p_owner;
    }
};