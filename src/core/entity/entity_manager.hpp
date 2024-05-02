#pragma once

#include <vector>
#include "entity/ant.hpp"
#include "entity/building.hpp"
#include "app/arg_parse.hpp"
#include "entity/map.hpp"
#include "ui/colors.hpp"
#include "hardware/controller.hpp"

struct EntityManager {
    Player player;
    std::vector<MapEntity*> ants;
    std::vector<Building*> buildings;
    MapBuilder builder;
    Map map;

    EntityManager(int map_width, int map_height, ProjectArguments& config)
        : player(MapData(40, 25, 10, '@', color::white)),
            builder(map_width, map_height),
            map(builder, config) {

        ants.push_back(&player);
    }

    ~EntityManager() {
        for(auto ant : ants) delete ant;
        for(auto building : buildings) delete building;
    }

    void compute_fov() {
        for (MapEntity* ant : ants) {
            MapData const& data = ant->get_data();
            map.compute_fov(data.x, data.y, data.fov_radius);
        }
        map.update_fov();
    }

    void create_ant(std::vector<ClockController*>& controllers, std::vector<std::string>& lines) {
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
        if (b == nullptr) {
            SPDLOG_DEBUG("No building found at player location");
            return;
        }

        long new_x = b->x + b->w / 2, new_y = b->y + b->h / 2;
        if (!map.can_place(new_x, new_y)) return;

        SPDLOG_DEBUG("Selected building at ({}, {})", new_x, new_y);

        SPDLOG_DEBUG("Creating worker controller");
        ParserCommandsAssembler assembler;
        Worker* new_ant = new Worker(MapData(new_x, new_y, 'w', 10, color::light_green));
        Worker_Controller* w =
            new Worker_Controller(assembler, new_ant->cpu, *new_ant, map, lines);
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