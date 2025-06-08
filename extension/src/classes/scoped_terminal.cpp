#include "scoped_terminal.hpp"
#include <godot_cpp/core/class_db.hpp>

void ScopedTerminal::_bind_methods() {
    ClassDB::bind_method(D_METHOD("get_value"), &ScopedTerminal::get_value);
    ClassDB::bind_method(D_METHOD("release_lock"), &ScopedTerminal::release_lock);
}

ScopedTerminal::ScopedTerminal(Terminal* val) {
    UtilityFunctions::print("Constructor");
    value = val;
}

ScopedTerminal::~ScopedTerminal() {
    UtilityFunctions::print("Destructor");
    if (value != nullptr) {
        ERR_FAIL_MSG("A Scoped Terminal was not given back");
    }
}

void ScopedTerminal::set_terminal(Terminal* terminal) {
    value = terminal;
}

Terminal* ScopedTerminal::get_value() const {
    return value;
}

void ScopedTerminal::release_lock() {
    UtilityFunctions::print("Released");
    value = nullptr;
}