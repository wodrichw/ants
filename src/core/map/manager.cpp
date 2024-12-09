#include "map/manager.hpp"
#include "map/map.hpp"
#include "map/world.hpp"

MapManager::MapManager(int map_section_width,
        int map_section_height,
        ProjectArguments& config,
        MapWorld& map_world
):
    is_walls_enabled(config.is_walls_enabled),
    map_section_width(map_section_width),
    map_section_height(map_section_height),
    map_world(map_world)
{
    map_world.regions.build_section(0,0, map_world.current_level());
}


MapManager::MapManager(const ant_proto::MapManager& msg, MapWorld& map_world):
    is_walls_enabled(msg.is_walls_enabled()),
    map_section_width(msg.map_section_width()),
    map_section_height(msg.map_section_height()),
    map_world(map_world)
{
    map_world.regions.build_section(0,0, map_world.current_level());
}


bool MapManager::update_current_level(const EntityData& d) {
    Map& cur_map = map_world.current_level().map;
    MapWindow& map_window = map_world.map_window;

    if( cur_map.needs_update == false ) return false;
    cur_map.needs_update = false;

    map_window.set_center(d.x, d.y);
    cur_map.update_chunks(map_window.border);

    return true;
}

void MapManager::update_fov(const EntityData& d) {
    map_world.map_window.compute_fov(d.x, d.y, d.fov_radius);
    for(long x = map_world.map_window.border.x1; x < map_world.map_window.border.x2; x++) {
        for(long y = map_world.map_window.border.y1; y < map_world.map_window.border.y2;
            y++) {
            if(!map_world.map_window.in_fov(x, y)) continue;

            map_world.current_level().map.explore(x, y);
        }
    }
}


void MapManager::update_map_window_tiles() {
    for(long x = map_world.map_window.border.x1; x < map_world.map_window.border.x2; x++) {
        for(long y = map_world.map_window.border.y1; y < map_world.map_window.border.y2; y++) {
            map_world.current_level().map.reset_tile(x, y);
        }
    }
}

void MapManager::generate_sections(ulong depth, const std::vector<ChunkMarker>& chunks) {
    for(const auto& chunk: chunks)
        map_world.regions.build_section(chunk.x, chunk.y, map_world.levels[depth]);
}


void MapManager::set_window_tiles() {
    SPDLOG_TRACE("Setting window tiles");
    Map& cur_map = map_world.current_level().map;
    MapWindow& map_window = map_world.map_window;
    
    for(long local_x = 0; local_x < map_window.border.w; local_x++) {
        for(long local_y = 0; local_y < map_window.border.h; local_y++) {
            long x = local_x + map_window.border.x1,
                 y = local_y + map_window.border.y1;
            if(cur_map.is_wall(x, y)) {
                map_window.set_wall(x, y);
            } else {
                map_window.set_floor(x, y);
            }
        }
    }
}


void MapManager::move_entity(ulong entity_depth, long dy, long dx, MapEntity& entity) {

    std::vector<ChunkMarker> added_chunks;
    bool successful_move  = map_world[entity_depth].map.move_entity(entity, dx, dy);
    if (successful_move) generate_sections(entity_depth, added_chunks);
}


bool MapManager::go_up() {
    if (map_world.current_depth == 0) return false;
    --map_world.current_depth;
    map_world.current_level().map.update_chunks(map_world.map_window.border);
    return true;
}


bool MapManager::go_down() {
    if (map_world.current_depth == map_world.levels.size()) {
        /* do nothing */
        // map_world.levels.emplace_back( Level(Map(border, is_walls_enabled), ) );
        // Map& new_map = map_world.levels[map_world.current_depth].map;
        // MapSectionData section;
        // RandomMapBuilder(Rect::from_top_left(0, 0, map_section_width, map_section_height))(
        //     section);
        // new_map.load_section(section);
    } else {
        ++map_world.current_depth;
    }

    map_world.current_level().map.update_chunks(map_world.map_window.border);
    return true;
}


ant_proto::MapManager MapManager::get_proto()  const {
    ant_proto::MapManager msg;
    msg.set_is_walls_enabled(is_walls_enabled);
    msg.set_map_section_width(map_section_width);
    msg.set_map_section_height(map_section_height);

    return msg;
}

