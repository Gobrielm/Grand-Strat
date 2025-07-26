#pragma once

#include <godot_cpp/classes/node.hpp>

using namespace godot;

class FactoryCreator : public Node {
    GDCLASS(FactoryCreator, Node);

private:
    static FactoryCreator* singleton_instance;

protected:
    static void _bind_methods();

public:
    FactoryCreator();
    ~FactoryCreator();
    
    static void create();
    static FactoryCreator* get_instance();

    void create_primary_industry(int type, Vector2i coords, int player_id, int mult = 1);
};
