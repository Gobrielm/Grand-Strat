#pragma once

#include <godot_cpp/variant/vector2i.hpp>
#include <godot_cpp/classes/weak_ref.hpp>

using namespace godot;

struct TownCargo {

    TownCargo(int p_type, int p_amount, float p_price, int p_terminal_id);
    int type;
    int amount;
    int age = 0;
    float price;
    int terminal_id;

    void sell_cargo(int p_amount);
    void transfer_cargo(int p_amount);

    // Sorts by price
    bool operator>(const TownCargo& other) const;
    bool operator==(const TownCargo& other) const;

    struct TownCargoPtrCompare {
        bool operator()(const TownCargo* lhs, const TownCargo* rhs) const {
            return lhs->price > rhs->price; // Min-heap: lowest price at top
        }
    };
};