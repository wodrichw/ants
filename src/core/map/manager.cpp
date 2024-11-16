#include "map/manager.hpp"
#include "map/builder.hpp"
#include "map/section_data.hpp"
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
    map_world.levels.emplace_back( Level(Map(border, is_walls_enabled)) );

    // Initialize initial map and that map's initial section
    MapSectionData section;
    if(config.default_map_file_path.empty()) {
        RandomMapBuilder(Rect::from_top_left(0, 0, map_section_width, map_section_height))(
            section);
    } else {
        FileMapBuilder(config.default_map_file_path)(section);
    }

    Map& initial_map = map_world.current_level().map;
    initial_map.load_section(section);

    
    SPDLOG_DEBUG("Loaded map section data");

    if(section.rooms.empty()) {
        SPDLOG_ERROR("No rooms found in the map section data");
        return;
    }

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

bool MapManager::go_up() {
    if (map_world.current_depth == 0) return false;
    --map_world.current_depth;
    map_world.current_level().map.update_chunks(map_world.map_window.border);
    return true;
}

bool MapManager::go_down() {
    ++map_world.current_depth;
    if (map_world.current_depth == map_world.levels.size()) {
        map_world.levels.emplace_back( Level(Map(border, is_walls_enabled)) );
        Map& new_map = map_world.levels[map_world.current_depth].map;
        MapSectionData section;
        RandomMapBuilder(Rect::from_top_left(0, 0, map_section_width, map_section_height))(
            section);
        new_map.load_section(section);
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

