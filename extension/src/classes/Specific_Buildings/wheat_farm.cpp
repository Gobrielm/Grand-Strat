#include "wheat_farm.hpp"
#include <godot_cpp/core/class_db.hpp>
#include "../../singletons/cargo_info.hpp"
#include "../../singletons/recipe_info.hpp"

void WheatFarm::_bind_methods() {
    ClassDB::bind_static_method(get_class_static(), D_METHOD("create", "coords", "p_player_id"), &WheatFarm::create);

    ClassDB::bind_method(D_METHOD("day_tick"), &WheatFarm::day_tick);
    ClassDB::bind_method(D_METHOD("month_tick"), &WheatFarm::month_tick);
}

WheatFarm::WheatFarm(): AiFactory() {}

WheatFarm::WheatFarm(Vector2i coords, int p_player_id): AiFactory(coords, p_player_id, get_recipe()) {}

Ref<WheatFarm> WheatFarm::create(Vector2i coords, int p_player_id) {
    return Ref<WheatFarm>(memnew(WheatFarm(coords, p_player_id)));
}

Recipe* WheatFarm::get_recipe() {
    return RecipeInfo::get_instance()->get_primary_recipe_for_type(CargoInfo::get_instance()->get_cargo_type("grain"));
}

void WheatFarm::day_tick() {
    AiFactory::day_tick();
}
void WheatFarm::month_tick() {
    AiFactory::month_tick();
}