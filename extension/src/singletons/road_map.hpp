#pragma once

#include <unordered_map>
#include <unordered_set>
#include <godot_cpp/classes/tile_map_layer.hpp>
#include <godot_cpp/classes/resource_loader.hpp>
#include <godot_cpp/classes/tile_set_atlas_source.hpp>
#include "../utility/vector2i_hash.hpp"

using namespace godot;

class RoadMap : public TileMapLayer {
    GDCLASS(RoadMap, TileMapLayer);

private:
    static RoadMap* singleton_instance;
    mutable std::mutex m;
    std::unordered_map<Vector2i, int, godot_helpers::Vector2iHasher> road_value;
    std::unordered_map<Vector2i, int, godot_helpers::Vector2iHasher> temp_road_value;
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
    void hover_road(Vector2i location);
    void upgrade_road(Vector2i location);
    void hover_upgrade(Vector2i location);
    void remove_hovers();
    int get_road_value(Vector2i location) const;
    int get_temp_road_value(Vector2i location) const;
    void place_road_depot(Vector2i location);
    void bfs_and_connect(const Vector2i& tile1, const Vector2i& tile2);
};
