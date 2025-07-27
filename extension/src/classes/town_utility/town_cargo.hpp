#pragma once

#include <godot_cpp/variant/vector2i.hpp>

using namespace godot;

struct TownCargo {

    TownCargo(Vector2i p_source, int p_type, int p_amount, float p_price);
    int type;
    int amount;
    int age = 0;
    float price;
    Vector2i source;

    // Sorts by price
    bool operator>(const TownCargo& other) const;
    bool operator==(const TownCargo& other) const;

    struct TownCargoPtrCompare {
        bool operator()(const TownCargo* lhs, const TownCargo* rhs) const {
            return lhs->price > rhs->price; // Min-heap: lowest price at top
        }
    };
};