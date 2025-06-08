#pragma once


#include "terminal.hpp"
#include <godot_cpp/classes/object.hpp>

using namespace godot;

class ScopedTerminal : public Object {
    GDCLASS(ScopedTerminal, Object);

    Terminal* value;

    protected:
    static void _bind_methods();


    public:

    ScopedTerminal(Terminal* val = nullptr);
    ~ScopedTerminal();
    Terminal* get_value() const;
    void release_lock(); //Only used by the 'owner' of value when it returns back

};