#include <ranges>
#include <algorithm>

#include "map/manager.hpp"
#include "map/builder.hpp"
#include "map/map.hpp"
#include "map/section_data.hpp"
#include "map/world.hpp"

namespace rgs = std::ranges;

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
    map_world.levels.emplace_back( Level(Map(border, is_walls_enabled), map_world.levels.size()) );

    // Initialize initial map and that map's initial section
    MapSectionData section;
    if(config.default_map_file_path.empty()) {
        RandomMapBuilder(Rect::from_top_left(0, 0, map_section_width, map_section_height))
            (section);
    } else {
        FileMapBuilder(config.default_map_file_path)
            (section);
    }

    Map& initial_map = map_world.current_level().map;
    initial_map.load_section(section);

    
    SPDLOG_DEBUG("Loaded map section data");
    SPDLOG_DEBUG("Creating nursery building");
    first_room = section.rooms[0];

    int center_x = get_first_room().center_x;
    int center_y = get_first_room().center_y;
    map_world.current_level().add_building(Nursery(center_x - 1, center_y - 1, 0));
    SPDLOG_INFO("Moving player to nursery building: ({}, {})", center_x,
                center_y);
    SPDLOG_DEBUG("Adding player entity to the map");
}


MapManager::MapManager(const ant_proto::MapManager& msg, MapWorld& map_world):
    is_walls_enabled(msg.is_walls_enabled()),
    map_section_width(msg.map_section_width()),
    map_section_height(msg.map_section_height()),
    first_room(msg.first_room()),
    map_world(map_world)
{

    SPDLOG_DEBUG("Unpacking MapManager, is_walls_enabled {}", is_walls_enabled);
    SPDLOG_DEBUG("Unpacking MapManager, map_section_width {}", map_section_width);
    SPDLOG_DEBUG("Unpacking MapManager, map_section_height {}", map_section_height);
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
    const auto chunks_compare = [](const ChunkMarker& a, const ChunkMarker& b){ return a.id < b.id; };
    const auto max_chunk = rgs::max_element(chunks, chunks_compare);
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


const Rect& MapManager::get_first_room() {
    return first_room;
}


ant_proto::MapManager MapManager::get_proto()  const {
    ant_proto::MapManager msg;
    msg.set_is_walls_enabled(is_walls_enabled);
    msg.set_map_section_width(map_section_width);
    msg.set_map_section_height(map_section_height);

    return msg;
}

