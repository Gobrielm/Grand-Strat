#include "road_map.hpp"
#include "terminal_map.hpp"
#include <queue>

RoadMap* RoadMap::singleton_instance = nullptr;

void RoadMap::_bind_methods() {
    ClassDB::bind_static_method(get_class_static(), D_METHOD("get_instance"), &RoadMap::get_instance);
    
    ClassDB::bind_method(D_METHOD("place_road_depot", "location"), &RoadMap::place_road_depot);

    ClassDB::bind_method(D_METHOD("place_road", "location"), &RoadMap::place_road);
    ClassDB::bind_method(D_METHOD("upgrade_road", "location"), &RoadMap::upgrade_road);
    ClassDB::bind_method(D_METHOD("get_road_value", "location"), &RoadMap::get_road_value);

    ClassDB::bind_method(D_METHOD("hover_road", "location"), &RoadMap::hover_road);
    ClassDB::bind_method(D_METHOD("hover_upgrade", "location"), &RoadMap::hover_upgrade);
    ClassDB::bind_method(D_METHOD("remove_hovers"), &RoadMap::remove_hovers);

    ClassDB::bind_method(D_METHOD("bfs_and_connect", "tile1", "tile2"), &RoadMap::bfs_and_connect);
    
}

void RoadMap::_notification(int p_what) {
    if (p_what == NOTIFICATION_READY) {
        ERR_FAIL_COND_MSG(singleton_instance != nullptr, "RoadMap has already been created but is being created again");
        singleton_instance = this;
    } else if (p_what == NOTIFICATION_EDITOR_POST_SAVE) {
        singleton_instance = nullptr;
    } else if (p_what == NOTIFICATION_EXIT_TREE) {
        singleton_instance = nullptr;
    }
}

RoadMap::RoadMap() {
    Ref<Texture2D> texture = ResourceLoader::get_singleton()->load("res://Map_Icons/roads.png");
    if (!texture.is_valid()) {
        UtilityFunctions::print("Failed to load tileset texture!");
        return;
    }

    tile_set.instantiate();
    tile_set->set_tile_shape(TileSet::TILE_SHAPE_HEXAGON);
    tile_set->set_tile_offset_axis(TileSet::TILE_OFFSET_AXIS_VERTICAL);
    tile_set -> set_tile_size(Vector2i(127, 110));
    set_tile_set(tile_set);

    set_z_as_relative(true);
    set_y_sort_enabled(true);

    atlas_source.instantiate();
    int source_id = tile_set->add_source(atlas_source);
    atlas_source -> set_texture_region_size(Vector2i(127, 128));
    atlas_source->set_texture(texture);
    atlas_source->set_texture_region_size(Vector2i(128, 128));
    atlas_source->set_separation(Vector2i(0, 64));
    
    atlas_source->create_tile(Vector2i(0, 5)); //For all 6 tile
    atlas_source->create_tile(Vector2i(1, 5)); //For road depot
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
}

RoadMap::~RoadMap() {
    singleton_instance = nullptr;
}

RoadMap* RoadMap::get_instance() {
    ERR_FAIL_COND_V_MSG(singleton_instance == nullptr, nullptr, "RoadMap has not been created but is being accessed");
    return singleton_instance;
}

void RoadMap::hover_road(Vector2i location) {
    m.lock();
    temp_road_value[location] = 1;
    m.unlock();
    fix_tile(location, true);
}

void RoadMap::place_road(Vector2i location) {
    m.lock();
    road_value[location] = 1;
    m.unlock();
    fix_tile(location, true);
}

void RoadMap::upgrade_road(Vector2i location) {
    m.lock();
    road_value[location] += 1;
    m.unlock();
    fix_tile(location, true);
}

void RoadMap::hover_upgrade(Vector2i location) {
    m.lock();
    temp_road_value[location] += 1;
    m.unlock();
    fix_tile(location, true);
}

int RoadMap::get_road_value(Vector2i location) const {
    std::scoped_lock lock(m);
    return road_value.count(location) == 0 ? 0: road_value.at(location);
}

int RoadMap::get_temp_road_value(Vector2i location) const {
    std::scoped_lock lock(m);
    return temp_road_value.count(location) == 0 ? 0: temp_road_value.at(location);
}

void RoadMap::fix_tile(Vector2i center, bool repeating) {
    
    Array tiles = get_surrounding_cells(center); //Starts at 2
    int offset = 4;
    int y = -1; //Also is 1 less than the number of connections
    std::vector<bool> connections = {};
    for (int i = 0; i < 6; i++) {
        if (tiles[i].get_type() != Variant::VECTOR2I) continue;
        Vector2i tile = tiles[(i + offset) % 6];
        int val = get_road_value(tile) + get_temp_road_value(tile);
        if (val > 0) {
            if (repeating) fix_tile(tile);
            y += 1;
            connections.push_back(true);
        } else {
            connections.push_back(false);
        }
    }
    if (get_cell_atlas_coords(center) == Vector2i(1, 5)) return;
    

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
    std::scoped_lock lock(m);
    set_cell(center, 0, Vector2i(index, y));
}

void RoadMap::remove_hovers() {
    for (const auto &[tile, road_val]: temp_road_value) {
        temp_road_value.erase(tile);
        fix_tile(tile, true);
        m.lock();
        erase_cell(tile);
        m.unlock();
    }
}

void RoadMap::place_road_depot(Vector2i location) {
    m.lock();
    set_cell(location, 0, Vector2i(1, 5));
    m.unlock();
    place_road(location);
}

void RoadMap::bfs_and_connect(const Vector2i& tile1, const Vector2i& tile2) {
    std::queue<Vector2i> q1;
    std::queue<Vector2i> q2;
    std::unordered_set<Vector2i, godot_helpers::Vector2iHasher> s1;
    std::unordered_set<Vector2i, godot_helpers::Vector2iHasher> s2;
    std::unordered_map<Vector2i, Vector2i, godot_helpers::Vector2iHasher> tile_to_prev1;
    std::unordered_map<Vector2i, Vector2i, godot_helpers::Vector2iHasher> tile_to_prev2;
    q1.push(tile1);
    q2.push(tile2);
    s1.insert(tile1);
    s2.insert(tile2);
    Vector2i curr;
    bool alternater = true;
    bool found = false;
    while (q1.size() != 0 && q2.size() != 0 && !found) {
        if (s1.size() > 10000) return;
        alternater = !alternater;
        if (alternater) {
            curr = q1.front();
            q1.pop();
        } else {
            curr = q2.front();
            q2.pop();
        }
        
        Array tiles = get_surrounding_cells(curr);
        for (int i = 0; i < tiles.size(); i++) {
            if (tiles[i].get_type() != Variant::VECTOR2I) continue;
            Vector2i tile = tiles[i];
            if (!TerminalMap::get_instance() -> is_tile_traversable(tile, false)) continue;
            if ((alternater && s1.count(tile)) || (!alternater && s2.count(tile))) continue;


            if (alternater) {
                q1.push(tile);
                s1.insert(tile);
                tile_to_prev1[tile] = curr;
                if (s2.count(tile)) {
                    found = true;
                    curr = tile;
                    break;
                }
            } else {
                q2.push(tile);
                s2.insert(tile);
                tile_to_prev2[tile] = curr;
                if (s1.count(tile)) {
                    found = true;
                    curr = tile;
                    break;
                }
            }
        }
    }
    if (found) {
        Vector2i end = curr;
        while (tile_to_prev1.count(curr)) {
            place_road(curr);
            curr = tile_to_prev1[curr];
            
        }
        curr = end;
        while (tile_to_prev2.count(curr)) {
            place_road(curr);
            curr = tile_to_prev2[curr];
        }
    }
}