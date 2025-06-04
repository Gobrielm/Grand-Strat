#pragma once

#include <unordered_map>
#include <godot_cpp/classes/tile_map_layer.hpp>
#include <godot_cpp/classes/resource_loader.hpp>
#include <godot_cpp/classes/tile_set_atlas_source.hpp>
#include "../utility/vector2i_hash.hpp"

using namespace godot;

class RoadMap : public TileMapLayer {
    GDCLASS(RoadMap, TileMapLayer);

private:
    static RoadMap* singleton_instance;
    std::unordered_map<Vector2i, int, godot_helpers::Vector2iHasher> road_value;
    Ref<TileSet> tile_set = nullptr;
    Ref<TileSetAtlasSource> atlas_source = nullptr;
    void fix_tile(Vector2i center, bool repeating = false);

protected:
    static void _bind_methods();
    void _notification(int p_what);

public:
    RoadMap();
    ~RoadMap();

    static RoadMap* get_instance();
    void place_road(Vector2i location);
    void upgrade_road(Vector2i location);
    int get_road_value(Vector2i location) const;
    void place_road_depot(Vector2i location);
};
