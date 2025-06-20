#include "wheat_farm.hpp"
#include <godot_cpp/core/class_db.hpp>
#include "../../singletons/cargo_info.hpp"

void WheatFarm::_bind_methods() {
    ClassDB::bind_static_method(get_class_static(), D_METHOD("create", "coords", "p_player_id"), &WheatFarm::create);

    ClassDB::bind_method(D_METHOD("day_tick"), &WheatFarm::day_tick);
    ClassDB::bind_method(D_METHOD("month_tick"), &WheatFarm::month_tick);
}

WheatFarm::WheatFarm(): AiFactory() {}

WheatFarm::WheatFarm(Vector2i coords, int p_player_id): AiFactory(coords, p_player_id, Dictionary(), get_input_dict()) {
    set_pops_needed(26);
}

Ref<WheatFarm> WheatFarm::create(Vector2i coords, int p_player_id) {
    return Ref<WheatFarm>(memnew(WheatFarm(coords, p_player_id)));
}

Dictionary WheatFarm::get_input_dict() {
    Dictionary d;
    d[CargoInfo::get_instance()->get_cargo_type("grain")] = 1;
    return d;
}

void WheatFarm::day_tick() {
    AiFactory::day_tick();
}
void WheatFarm::month_tick() {
    AiFactory::month_tick();
}