#include <ranges>
#include "map/manager.hpp"
#include "app/globals.hpp"
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
    set_window_tiles();
    cur_map.update_chunks(map_window.border);

    return true;
}

void MapManager::update_fov(const EntityData& d) {
    map_world.map_window.compute_fov(d.x, d.y, d.fov_radius);
    for(long x = map_world.map_window.border.x1; x < map_world.map_window.border.x2; x++) {
        for(long y = map_world.map_window.border.y1; y < map_world.map_window.border.y2; y++) {
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


void MapManager::generate_sections(ulong depth, const std::vector<ChunkMarker>& chunks) {
    Map& map = map_world[depth].map;
    auto unbuilt_chunks = chunks 
        | std::views::filter([&map](const ChunkMarker& cm){ return !map.chunk_built(cm); });

    for(const auto& chunk: unbuilt_chunks)
        map_world.regions.build_section(chunk.x, chunk.y, map_world.levels[depth]);
}


void MapManager::move_entity(ulong entity_depth, long dy, long dx, MapEntity& entity) {
    Map& map = map_world[entity_depth].map;
    bool successful_move  = map.move_entity(entity, dx, dy);
    // if (successful_move) {
    //     //inspect for freshly added chunks and generate sections if necessary
    //     EntityData& ed = entity.get_data();
    //     ulong cl = globals::CHUNK_LENGTH;
    //     std::vector<ChunkMarker> added_chunks = map.get_chunk_markers(Rect(ed.x - cl, ed.y - cl, cl*2, cl*2));
    //     added_chunks | std::views::filter([&map](const ChunkMarker& cm){ return map.chunk_built(cm); });
    //     generate_sections(entity_depth, added_chunks);
    // }
}


bool MapManager::go_up() {
    if (map_world.current_depth == 0) return false;
    --map_world.current_depth;
    map_world.current_level().map.update_chunks(map_world.map_window.border);
    return true;
}


bool MapManager::go_down() {
    if (map_world.current_depth < map_world.levels.size() - 1) {
        ++map_world.current_depth;
        map_world.current_level().map.update_chunks(map_world.map_window.border);
        return true;
    }
    return false;
}


ant_proto::MapManager MapManager::get_proto()  const {
    ant_proto::MapManager msg;
    msg.set_is_walls_enabled(is_walls_enabled);
    msg.set_map_section_width(map_section_width);
    msg.set_map_section_height(map_section_height);

    return msg;
}

