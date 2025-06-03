// vector2i_hash.hpp
#pragma once
#include <godot_cpp/variant/vector2i.hpp>
#include <functional>

namespace godot_helpers {
    struct Vector2iHasher {
        size_t operator()(const godot::Vector2i& v) const {
            return std::hash<int>()(v.x) ^ (std::hash<int>()(v.y) << 1);
        }
    };
}