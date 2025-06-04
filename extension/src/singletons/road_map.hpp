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
    Ref<TileSet> tileset;
    Ref<TileSetAtlasSource> atlas_source;
    void fix_tile(Vector2i center, bool repeating = false);

protected:
    static void _bind_methods();

public:
    RoadMap();
    ~RoadMap();


    void place_road(Vector2i location);
    void upgrade_road(Vector2i location);
    int get_road_value(Vector2i location) const;
    

};
