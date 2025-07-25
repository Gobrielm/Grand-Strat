#pragma once

#include "../ai_factory.hpp"

using namespace godot;

class WheatFarm : public AiFactory {
    GDCLASS(WheatFarm, AiFactory)


protected:
    static void _bind_methods();
    Recipe* get_recipe();

public:
    WheatFarm();
    WheatFarm(Vector2i coords, int p_player_id);
    static Ref<WheatFarm> create(Vector2i coords, int p_player_id);

    void day_tick() override;
    void month_tick() override;
};
