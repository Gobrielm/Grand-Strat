#include "town_cargo.hpp"

TownCargo::TownCargo(Vector2i p_source, int p_type, int p_amount, float p_price) {
    source = p_source;
    type = p_type;
    amount = p_amount;
    price = p_price;
}