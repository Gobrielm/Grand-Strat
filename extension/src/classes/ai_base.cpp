#include "ai_base.hpp"
#include "../singletons/province_manager.hpp"
#include "../singletons/terminal_map.hpp"
#include "../singletons/road_map.hpp"
#include "../singletons/factory_creator.hpp"

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

bool AiBase::is_tile_owned(Vector2i tile) const {
    Ref<ProvinceManager> province_manager = ProvinceManager::get_instance();
    return province_manager -> get_province(province_manager -> get_province_id(tile)) -> get_country_id() == get_country_id();
}   

void AiBase::place_depot(Vector2i tile) {
    RoadMap::get_instance()->place_road_depot(tile);
	FactoryCreator::get_instance()->create_road_depot(tile, get_owner_id());
}