#pragma once

#include <godot_cpp/variant/vector2i.hpp>
#include <list>

using namespace godot;

class OwnerComponent {
    private:

    public:
    int owner;

    inline OwnerComponent(int p_owner = 0) {
        owner = p_owner;
    }

    inline bool is_owner_a_player() const {
        return owner > 0;
    }

    inline bool is_owner_a_company() const {
        return owner < 0;
    }

    inline bool is_owner_an_ai() const {
        return owner == 0;
    }
};