#pragma once

#include <godot_cpp/classes/object.hpp>

using namespace godot;

class Terminal : public Object {
    GDCLASS(Terminal, Object);
    Vector2i location;
    int player_owner;
    
    protected:
    static void _bind_methods();
    

    public:
    void set_location(const Vector2i p_location);
    Vector2i get_location() const;
    int get_player_owner() const;
    static Terminal* create(const Vector2i p_location, const int p_owner);
    
    virtual void initialize(const Vector2i p_location = Vector2i(0, 0), const int p_owner = 0);

    Terminal() {}
    Terminal(const Vector2i p_location, const int p_owner) {
        location = p_location;
        player_owner = p_owner;
    }
    ~Terminal() {}
};