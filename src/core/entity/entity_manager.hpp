#pragma once

#include <vector>

#include "app/arg_parse.hpp"
#include "entity/ant.hpp"
#include "entity/building.hpp"
#include "entity/map.hpp"
#include "entity/map_builder.hpp"
#include "entity/map_section_data.hpp"
#include "entity/map_window.hpp"
#include "hardware/controller.hpp"
#include "ui/colors.hpp"

struct EntityManager {
    Player player;
    std::vector<MapEntity*> ants;
    std::vector<Building*> buildings;
    MapWindow map_window;
    Map map;

    EntityManager(int map_width, int map_height, ProjectArguments& config)
        : player(EntityData(40, 25, '@', 10, color::white)),
          map_window(Rect::from_center(player.get_data().x, player.get_data().y,
                                       map_width, map_height)),
          map(map_window.border, config.is_walls_enabled) {
        ants.push_back(&player);

        MapSectionData section;
        if(config.default_map_file_path.empty()) {
            RandomMapBuilder(Rect::from_top_left(0, 0, map_width, map_height))(
                section);
        } else {
            FileMapBuilder(config.default_map_file_path)(section);
        }
        map.load_section(section);
        SPDLOG_DEBUG("Loaded map section data");

        if(section.rooms.empty()) {
            SPDLOG_ERROR("No rooms found in the map section data");
            return;
        }

        SPDLOG_DEBUG("Creating nursery building");
        Rect& first_room = section.rooms[0];
        int center_x = first_room.center_x, center_y = first_room.center_y;

        buildings.push_back(new Nursery(center_x - 1, center_y - 1, 0));
        SPDLOG_INFO("Moving player to nursery building: ({}, {})", center_x,
                    center_y);
        player.data.x = center_x, player.data.y = center_y;
        map.add_building(*buildings[0]);

        SPDLOG_DEBUG("Adding player entity to the map");
        map.add_entity(player);
        map_window.set_center(player.get_data().x, player.get_data().y);
    }

    ~EntityManager() {
        SPDLOG_DEBUG("Destroying EntityManager");
        for(auto ant : ants) delete ant;
        for(auto building : buildings) delete building;
        SPDLOG_TRACE("Destroyed ants and buildings");
    }

    void update_fov() {
        // SPDLOG_TRACE("Updating FOV");
        // map.reset_fov();
        for(long x = map_window.border.x1; x < map_window.border.x2; x++) {
            for(long y = map_window.border.y1; y < map_window.border.y2; y++) {
                map.reset_tile(x, y);
            }
        }
        for(auto ant : ants) {
            EntityData& d = ant->get_data();

            map_window.compute_fov(d.x, d.y, d.fov_radius);
            for(long x = map_window.border.x1; x < map_window.border.x2; x++) {
                for(long y = map_window.border.y1; y < map_window.border.y2;
                    y++) {
                    if(!map_window.in_fov(x, y)) continue;

                    map.explore(x, y);
                }
            }
        }
        // SPDLOG_TRACE("FOV updated");
    }

    void set_window_tiles() {
        SPDLOG_TRACE("Setting window tiles");
        long wall_count = 0, floor_count = 0;
        for(long local_x = 0; local_x < map_window.border.w; local_x++) {
            for(long local_y = 0; local_y < map_window.border.h; local_y++) {
                long x = local_x + map_window.border.x1,
                     y = local_y + map_window.border.y1;
                // SPDLOG_TRACE("Setting tile at ({}, {}) - local ({}, {})", x,
                // y, local_x, local_y);

                if(map.is_wall(x, y)) {
                    map_window.set_wall(x, y);
                    wall_count++;
                } else {
                    map_window.set_floor(x, y);
                    floor_count++;
                }
            }
        }
        SPDLOG_TRACE("Window tiles set - walls: {}, floors: {}", wall_count,
                     floor_count);
    }

    void update() {
        if(!map.needs_update) return;
        SPDLOG_TRACE("Updating EntityManager");
        map.needs_update = false;

        map_window.set_center(player.get_data().x, player.get_data().y);
        map.update_chunks(map_window.border);
        set_window_tiles();
        update_fov();
    }

    void create_ant(std::vector<ClockController*>& controllers,
                    std::vector<std::string>& lines) {
        // if(key_sym == SDLK_a && player->bldgId.has_value()) {
        // Make worker
        // TODO: make an intelligent location picker for workers
        // (addAnt_x,addAnt_y)
        //   -- should look around building and scan for an open location
        //   -- if no location present increase the scan radius (as if the
        //   building absorbs its perimiter squares)
        //      and look for open squared there. Radius increasing will go on
        //      until an open square is found (or out of space in the map)

        SPDLOG_INFO("Detected 'a' key press, adding worker ant");
        Building* b = map.get_building(player);
        if(b == nullptr) {
            SPDLOG_DEBUG("No building found at player location");
            return;
        }

        long new_x = b->x + b->w / 2, new_y = b->y + b->h / 2;
        if(!map.can_place(new_x, new_y)) return;

        SPDLOG_DEBUG("Selected building at ({}, {})", new_x, new_y);

        SPDLOG_DEBUG("Creating worker controller");
        CommandMap assembler;
        Worker* new_ant =
            new Worker(EntityData(new_x, new_y, 'w', 10, color::light_green));
        Worker_Controller* w = new Worker_Controller(assembler, new_ant->cpu,
                                                     *new_ant, map, lines);
        if(w->parser.status.p_err) {
            // TODO: show parse errors in the text editor box instead of a cout
            // this will likely require returning the line number, and word that
            // caused the parse error
            SPDLOG_ERROR("Error parsing worker controller: {}",
                         w->parser.status.err_msg);
            delete new_ant;
            delete w;
            return;
        }

        SPDLOG_DEBUG("Creating worker ant");
        controllers.push_back(w);

        map.add_entity(*new_ant);
        ants.push_back(new_ant);

        return;
    }
};
