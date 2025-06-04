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

    unsigned long long factorial(int n);
    unsigned long long nCr(int n, int r);

    inline unsigned long long nCr(int n, int r) {
        if (r > n) return 0;
        return factorial(n) / (factorial(r) * factorial(n - r));
    }

    inline unsigned long long factorial(int n) {
        if (n == 0 || n == 1) return 1;
        unsigned long long result = 1;
        for (int i = 2; i <= n; i++)
            result *= i;
        return result;
    }
}