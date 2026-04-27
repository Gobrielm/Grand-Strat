#pragma once

#include <godot_cpp/classes/object.hpp>

using namespace godot;

class MapObjectInfo: public Object {
    GDCLASS(MapObjectInfo, Object);

    protected:
    
    static void _bind_methods();

    public:

    MapObjectInfo();

    static Array get_town_factories(Vector2i town_tile);
    static Dictionary get_town_prices(Vector2i town_tile);

    static Dictionary get_factory_cargo_dictionary(const Vector2i coords);
    static int16_t get_cash_of_factory(const Vector2i coords);
};