#pragma once

#include <spdlog/spdlog.h>
#include <cassert>
#include "app/arg_parse.hpp"
#include "entity.pb.h"
#include "entity/map.hpp"
#include "entity/map_builder.hpp"
#include "entity/map_section_data.hpp"
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
    MapManager(ulong& current_depth, int map_section_width, int map_section_height, ProjectArguments& config, MapWindow& map_window):
        border(map_window.border),
        is_walls_enabled(config.is_walls_enabled),
        maps({{map_window.border, config.is_walls_enabled}}),
        current_depth(current_depth),
        map_section_width(map_section_width),
        map_section_height(map_section_height)
    {
        // Initialize initial map and that map's initial section
        MapSectionData section;
        if(config.default_map_file_path.empty()) {
            RandomMapBuilder(Rect::from_top_left(0, 0, map_section_width, map_section_height))(
                section);
        } else {
            FileMapBuilder(config.default_map_file_path)(section);
        }

        Map& initial_map = maps[current_depth];
        initial_map.load_section(section);

        
        SPDLOG_DEBUG("Loaded map section data");

        if(section.rooms.empty()) {
            SPDLOG_ERROR("No rooms found in the map section data");
            return;
        }

        SPDLOG_DEBUG("Creating nursery building");
        first_room = section.rooms[0];
    }

    MapManager(Unpacker& p, ulong& current_depth):
        border(p),
        is_walls_enabled(),
        maps(),
        current_depth(current_depth),
        map_section_width(),
        map_section_height(),
        first_room(p)
    {
        ant_proto::MapManager msg;
        p >> msg;
        is_walls_enabled = msg.is_walls_enabled();
        map_section_width = msg.map_section_width();
        map_section_height = msg.map_section_height();

        ulong max_depth = msg.max_depth();

        SPDLOG_DEBUG("Unpacking MapManager, map count {}", max_depth);
        SPDLOG_DEBUG("Unpacking MapManager, is_walls_enabled {}", is_walls_enabled);
        SPDLOG_DEBUG("Unpacking MapManager, map_section_width {}", map_section_width);
        SPDLOG_DEBUG("Unpacking MapManager, map_section_height {}", map_section_height);

        for(ulong i = 0; i < max_depth; ++i) {
            maps.emplace_back(p, is_walls_enabled);
        }
    }

    void go_up() {
    }

    void go_down() {
        if( current_depth == maps.size() ) {
            maps.push_back(Map(border, is_walls_enabled));
            Map& new_map = maps[current_depth];
            MapSectionData section;
            RandomMapBuilder(Rect::from_top_left(0, 0, map_section_width, map_section_height))(
                section);
            new_map.load_section(section);
        }
    }

    const Rect& get_first_room() {
        return first_room;
    }

    Map& get_map(size_t depth) {
        assert(depth < maps.size());
        return maps[depth];
    }

    Map& get_map() {
        return maps[current_depth];
    }

    const Map& get_map() const {
        return maps[current_depth];
    }

    friend Packer& operator<<(Packer& p, MapManager const& obj) {
        ant_proto::MapManager msg;
        msg.set_is_walls_enabled(obj.is_walls_enabled);
        msg.set_map_section_width(obj.map_section_width);
        msg.set_map_section_height(obj.map_section_height);
        msg.set_max_depth(obj.maps.size());
        p << obj.border << obj.first_room << msg;

        SPDLOG_DEBUG("Packing MapManager, map count {}", obj.maps.size());
        SPDLOG_DEBUG("Packing MapManager, is_walls_enabled {}", obj.is_walls_enabled);
        SPDLOG_DEBUG("Packing MapManager, map_section_width {}", obj.map_section_width);
        SPDLOG_DEBUG("Packing MapManager, map_section_height {}", obj.map_section_height);
        
        for(const auto& map: obj.maps) {
            p << map;
        }
        
        return p;
    }
};
