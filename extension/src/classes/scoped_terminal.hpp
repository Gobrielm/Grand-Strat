#pragma once


#include "terminal.hpp"
#include <godot_cpp/classes/ref_counted.hpp>

using namespace godot;

class ScopedTerminal : public RefCounted {
    GDCLASS(ScopedTerminal, RefCounted);

    Terminal* value;

    protected:
    static void _bind_methods();


    public:

    ScopedTerminal(Terminal* val = nullptr);
    ~ScopedTerminal();
    void set_terminal(Terminal* terminal);
    Terminal* get_value() const;
    void release_lock(); //Only used by the 'owner' of value when it returns back

};