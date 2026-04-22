#pragma once

#include "firm.hpp"
#include <mutex>
#include <atomic>

using namespace godot;

class FactoryTemplate;

class AiBase : public Firm {
    GDCLASS(AiBase, Firm);
    const int country_id;
    Vector2i stored_tile;

    protected:
    static void _bind_methods();
    static std::atomic<int> NUMBER_OF_AIs;
    mutable std::mutex m;

    public:

    AiBase();
    AiBase(int p_country_id, Vector2i tile);
    AiBase(int p_country_id, int p_owner_id, Vector2i tile);
    int get_country_id() const;
    int get_owner_id() const;
    Vector2i get_stored_tile() const;
    void set_stored_tile(Vector2i tile);

    //Utility Functions
    bool is_tile_owned(Vector2i tile) const;
};