#include "scoped_terminal.hpp"
#include <godot_cpp/core/class_db.hpp>

void ScopedTerminal::_bind_methods() {}


ScopedTerminal::ScopedTerminal(Terminal* val) {
    value = val;
}

ScopedTerminal::~ScopedTerminal() {
    if (value != nullptr) {
        ERR_FAIL_MSG("A Scoped Terminal was not given back");
    }
}

Terminal* ScopedTerminal::get_value() const {
    return value;
}

void ScopedTerminal::release_lock() {
    value = nullptr;
}