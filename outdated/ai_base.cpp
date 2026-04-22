#include "ai_base.hpp"
#include "../singletons/province_manager.hpp"

using namespace godot;

std::atomic<int> AiBase::NUMBER_OF_AIs = 0;

void AiBase::_bind_methods() {}

AiBase::AiBase() : country_id(-1), Firm() {}

AiBase::AiBase(int p_country_id, Vector2i tile): Firm(tile, (++NUMBER_OF_AIs) * -1), country_id(p_country_id) {}

AiBase::AiBase(int p_country_id, int p_owner_id, Vector2i tile): Firm(tile, p_owner_id), country_id(p_country_id) {}

int AiBase::get_country_id() const {
    return country_id;
}

int AiBase::get_owner_id() const {
    return player_owner;
}

Vector2i AiBase::get_stored_tile() const {
    std::scoped_lock lock(m);
    return stored_tile;
}

void AiBase::set_stored_tile(Vector2i tile) {
    std::scoped_lock lock(m);
    stored_tile = tile;
}

bool AiBase::is_tile_owned(Vector2i tile) const {
    Ref<ProvinceManager> province_manager = ProvinceManager::get_instance();
    auto province = province_manager -> get_province(tile);
    return province != nullptr && province -> get_country_id() == get_country_id();
}   