#pragma once

#include <spdlog/spdlog.h>
#include <cassert>
#include "app/arg_parse.hpp"
#include "entity.pb.h"
#include "entity/map.hpp"
#include "entity/map_window.hpp"

class MapManager {
    Rect border;
    bool is_walls_enabled;
    std::vector<Map> maps; // map collection ordered by depth
    ulong& current_depth; // defined in entity_manager
    ulong map_section_width;
    ulong map_section_height;
    Rect first_room;

public:
    MapManager(ulong& current_depth,
            int map_section_width,
            int map_section_height,
            ProjectArguments& config,
            MapWindow& map_window
    );

    MapManager(const ant_proto::MapManager& msg, ulong& current_depth);

    void go_up() { }
    void go_down();
    const Rect& get_first_room();
    Map& get_map(size_t depth);
    Map& get_map();
    const Map& get_map() const;
    ant_proto::MapManager get_proto()  const;
};
