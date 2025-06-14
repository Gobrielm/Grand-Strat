#pragma once

#include <godot_cpp/classes/object.hpp>

using namespace godot;

class FactoryTemplate;

class AiBase : public Object {
    GDCLASS(AiBase, Object);
    int country_id;
    int owner_id;
    Vector2i stored_tile;


    protected:
    static void _bind_methods();

    public:

    AiBase();
    AiBase(int p_country_id, int p_owner_id);
    int get_country_id() const;
    int get_owner_id() const;
    Vector2i get_stored_tile() const;
    void set_stored_tile(Vector2i tile);

    //Utility Functions
    bool is_tile_owned(Vector2i tile) const;
    void place_depot(Vector2i tile);
};