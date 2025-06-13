#include "ai_base.hpp"

using namespace godot;

void AiBase::_bind_methods() {}

AiBase::AiBase() : country_id(-1), owner_id(-1), stored_tile(Vector2i(0, 0)) {}

AiBase::AiBase(int p_country_id, int p_owner_id): country_id(p_country_id), owner_id(p_owner_id), stored_tile(Vector2i(0, 0)) {}

int AiBase::get_country_id() const {
    return country_id;
}

int AiBase::get_owner_id() const {
    return owner_id;
}

Vector2i AiBase::get_stored_tile() const {
    return stored_tile;
}

void AiBase::set_stored_tile(Vector2i tile) {
    stored_tile = tile;
}
