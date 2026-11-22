#pragma once

#include <godot_cpp/variant/vector2i.hpp>
#include <atomic>

using namespace godot;

enum class BuildingType: int {
    INVALID = -1,
    TOWN = 1,
    FACTORY = 2,
    STATION = 3,
    SUBSISTENCE_FARM = 4
};

class PositionComponent {
    private:
    inline static std::atomic<int> TOTAL_OBJECTS = 0;
    std::pair<int, int> position;
    int building_id;
    BuildingType type;

    public:
    

    PositionComponent(): building_id(-1), type(BuildingType::INVALID), position({0, 0}) {}

    PositionComponent(const PositionComponent& other): building_id(other.building_id), type(other.type), position(other.position) {}

    PositionComponent(const PositionComponent* other): building_id(other->building_id), type(other->type), position(other->position) {}

    PositionComponent(std::pair<int, int> p_position, BuildingType p_type): building_id(TOTAL_OBJECTS.fetch_add(1)), type(p_type), position(p_position) {}

    PositionComponent& operator=(const PositionComponent& other) {
        if (this == &other) {
            return *this;
        }

        position = other.position;
        building_id = other.building_id;
        type = other.type;
        
        return *this;
    }

    int get_building_id() const {
        return building_id;
    }

    BuildingType get_type() const {
        return type;
    }

    Vector2i get_position_vector2i() const {
        return Vector2i(position.first, position.second);
    }
};
