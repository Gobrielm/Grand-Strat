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
};