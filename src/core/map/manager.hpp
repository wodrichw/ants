#pragma once

#include <spdlog/spdlog.h>
#include <cassert>

#include "map.pb.h"
#include "app/arg_parse.hpp"
#include "map/map.hpp"
#include "map/world.hpp"

class Worker;
struct Building;

class MapManager {
    Rect border;
    bool is_walls_enabled;
    ulong map_section_width;
    ulong map_section_height;
    Rect first_room;
public:
    MapWorld& map_world;

    MapManager(int map_section_width,
        int map_section_height,
        ProjectArguments& config,
        MapWorld& map_world
    );
    MapManager(const ant_proto::MapManager& msg, MapWorld& map_world);


    bool update_current_level(const EntityData& d);
    void update_fov(const EntityData& d);
    void update_map_window_tiles();
    void generate_sections(ulong depth, const std::vector<ChunkMarker>& chunks);
    void set_window_tiles();
    void move_entity(ulong entity_depth, long dy, long dx, MapEntity& entity);
    bool go_up();
    bool go_down();
    std::vector<Building*> get_current_level_buildings();
    const Rect& get_first_room();
    ant_proto::MapManager get_proto()  const;
};
