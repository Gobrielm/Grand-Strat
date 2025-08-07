// vector2i_hash.hpp
#pragma once
#include <godot_cpp/variant/vector2i.hpp>
#include <functional>

namespace godot_helpers {
    struct Vector2iHasher {
        size_t operator()(const godot::Vector2i& v) const {
            size_t h1 = std::hash<int>()(v.x);
            size_t h2 = std::hash<int>()(v.y);
            return h1 ^ (h2 + 0x9e3779b9 + (h1 << 6) + (h1 >> 2));
        }
    };

    struct StringHasher {
        size_t operator()(const godot::String& v) const {
            std::string utf8 = std::string(v.utf8().get_data());
            return std::hash<std::string>()(utf8);
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

    template<typename T>
    struct weighted_value {
        float weight;
        T val;

        weighted_value(): val(), weight(-1) {}

        weighted_value(T p_val, float p_weight): val(p_val), weight(p_weight) {}

        bool operator==(const weighted_value &other) const {
            return weight == other.weight;
        }

        bool operator<(const weighted_value &other) const {
            return weight < other.weight;
        }

        bool operator>(const weighted_value &other) const {
            return weight > other.weight;
        }
    };
}