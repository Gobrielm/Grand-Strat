#pragma once

#include <godot_cpp/classes/ref_counted.hpp>
#include <shared_mutex>


using namespace godot;

class Terminal : public RefCounted {
    GDCLASS(Terminal, RefCounted);
    static std::atomic<int> total_terminals;
    Vector2i location;
    
    
    protected:
    static void _bind_methods();
    int player_owner;
    

    public:
    const int terminal_id;
    mutable std::shared_mutex m;
    void set_location(const Vector2i p_location);
    Vector2i get_location() const;
    int get_player_owner() const;
    int get_terminal_id() const;
    static Ref<Terminal> create(const Vector2i p_location, const int p_owner);
    
    

    virtual void initialize(const Vector2i p_location = Vector2i(0, 0), const int p_owner = 0);

    Terminal();
    Terminal(const Vector2i p_location, const int p_owner);
    virtual ~Terminal() {}
    Terminal(const Terminal&);
};