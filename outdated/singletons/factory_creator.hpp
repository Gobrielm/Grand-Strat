#pragma once

#include <godot_cpp/classes/object.hpp>

using namespace godot;

class Recipe;

class FactoryCreator : public Object {
    GDCLASS(FactoryCreator, Object);

private:
    static FactoryCreator* singleton_instance;

protected:
    static void _bind_methods();

public:
    FactoryCreator();
    ~FactoryCreator();
    
    static void create();
    static void cleanup();
    static FactoryCreator* get_instance();

    int create_primary_industry(int type, Vector2i coords, int player_id, int mult = 1);
    int create_primary_industry_no_cargo_map_call(int type, Vector2i coords, int player_id, int mult = 1);
    int create_road_depot(Vector2i coords, int player_id);
    int create_construction_site(Vector2i coords, int player_id);
    void create_town(Vector2i coords);
};
