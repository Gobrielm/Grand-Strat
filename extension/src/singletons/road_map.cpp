#include "road_map.hpp"

RoadMap* RoadMap::singleton_instance = nullptr;

void RoadMap::_bind_methods() {
    ClassDB::bind_method(D_METHOD("place_road", "location"), &RoadMap::place_road);
    ClassDB::bind_method(D_METHOD("upgrade_road", "location"), &RoadMap::upgrade_road);
    ClassDB::bind_method(D_METHOD("get_road_value", "location"), &RoadMap::get_road_value);
}

RoadMap::RoadMap() {
    set_z_as_relative(true);
    set_y_sort_enabled(true);

    Ref<Texture2D> texture = ResourceLoader::get_singleton()->load("res://Map_Icons/roads.png");
    if (!texture.is_valid()) {
        UtilityFunctions::print("Failed to load tileset texture!");
        return;
    }


    tileset.instantiate();
    tileset->set_tile_shape(TileSet::TILE_SHAPE_HEXAGON);
    tileset->set_tile_offset_axis(TileSet::TILE_OFFSET_AXIS_VERTICAL);
    tileset -> set_tile_size(Vector2i(127, 110));

    atlas_source.instantiate();
    int source_id = tileset->add_source(atlas_source);
    atlas_source -> set_texture_region_size(Vector2i(127, 128));
    atlas_source->set_texture(texture);
    atlas_source->set_texture_region_size(Vector2i(128, 128));
    atlas_source->set_separation(Vector2i(0, 64));
    


    atlas_source->create_tile(Vector2i(0, 5)); //For all 6 tile
    atlas_source->create_tile(Vector2i(6, 0)); //For extra 0 tile

    for (int i = 0; i < 20; i++) {
         atlas_source->create_tile(Vector2i(i, 2));
        if (i < 6) {
            atlas_source->create_tile(Vector2i(i, 0));
            atlas_source->create_tile(Vector2i(i, 4));
        }
        if (i < 15) {
            atlas_source->create_tile(Vector2i(i, 1));
            atlas_source->create_tile(Vector2i(i, 3));
        }
       
    }
    
    set_tile_set(tileset);
    singleton_instance = this;

}

RoadMap::~RoadMap() {}


void RoadMap::place_road(Vector2i location) {
    road_value[location] = 1;
    fix_tile(location, true);
}

void RoadMap::upgrade_road(Vector2i location) {
    road_value[location] += 1;
    fix_tile(location, true);
}

int RoadMap::get_road_value(Vector2i location) const {
    return road_value.count(location) == 0 ? 0: road_value.at(location);
}

void RoadMap::fix_tile(Vector2i center, bool repeating) {
    Array tiles = get_surrounding_cells(center); //Starts at 2
    int offset = 4;
    int y = -1; //Also is 1 less than the number of connections
    std::vector<bool> connections = {};
    for (int i = 0; i < 6; i++) {
        Vector2i tile = tiles[(i + offset) % 6];
        int val = get_road_value(tile);
        if (val > 0) {
            if (repeating) fix_tile(tile);
            y += 1;
            connections.push_back(true);
        } else {
            connections.push_back(false);
        }
    }
    

    if (y == -1) {
        set_cell(center, 0, Vector2i(6, 0)); //Just dot
        return;
    } else if (y == 5) {
        set_cell(center, 0, Vector2i(0, 5)); //5 tile
        return;
    }

    int uncounted_connections = y;
    int index = 0;
    int current = 0;
    for (bool val: connections) {
        if (val) {
            uncounted_connections -= 1;
            if (uncounted_connections == -1) break;
        } else {
            index +=  godot_helpers::nCr(5 - current, uncounted_connections);
        }
        current++;
    }
    set_cell(center, 0, Vector2i(index, y));
}