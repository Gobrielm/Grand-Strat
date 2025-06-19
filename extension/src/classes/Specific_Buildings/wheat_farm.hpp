#pragma once

#include "../ai_factory.hpp"

using namespace godot;

class WheatFarm : public AiFactory {
    GDCLASS(WheatFarm, AiFactory)


protected:
    static void _bind_methods();
    Dictionary get_input_dict();

public:
    WheatFarm();
    WheatFarm(Vector2i coords, int p_player_id);
    static Ref<WheatFarm> create(Vector2i coords, int p_player_id);
};
