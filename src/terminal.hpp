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
    

    void set_location(const Vector2i p_location);
    Vector2i get_location() const;
    int get_player_id() const;
    int calculate_reward(int type, int amount) const;
    static Terminal* create(const Vector2i p_location, const int p_owner);
    
    void initialize(const Vector2i p_location = Vector2i(), const int p_owner = 0);

    _FORCE_INLINE_ Terminal() {}
    _FORCE_INLINE_ Terminal(const Vector2i p_location, const int p_owner) {
        location = p_location;
        player_owner = p_owner;
    }
};