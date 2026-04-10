#pragma once

#include <godot_cpp/variant/vector2i.hpp>
#include <atomic>

using namespace godot;

class PositionComponent {
    private:
    static std::atomic<int> TOTAL_OBJECTS;

    public:
    std::pair<int, int> position;
    int building_id;

    PositionComponent() {
        position = {0, 0};
    }

    PositionComponent(std::pair<int, int> p_position) {
        position = p_position;
        building_id = TOTAL_OBJECTS++;
    }

    Vector2i get_position_vector2i() const {
        return Vector2i(position.first, position.second);
    }
};