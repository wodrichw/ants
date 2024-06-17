#pragma once

#include <vector>

#include "app/arg_parse.hpp"
#include "entity/ant.hpp"
#include "entity/building.hpp"
#include "entity/entity_data.hpp"
#include "entity/map.hpp"
#include "entity/map_builder.hpp"
#include "entity/map_section_data.hpp"
#include "entity/map_window.hpp"
#include "hardware/hardware_manager.hpp"
#include "hardware/software_manager.hpp"
#include "spdlog/spdlog.h"
#include "ui/colors.hpp"

struct EntityManager {
    ItemInfoMap item_info_map;
    Player player;
    std::vector<Worker*> workers;
    std::vector<Building*> buildings;
    MapWindow map_window;
    Map map;
    Worker* next_worker;
    ulong instr_action_clock;

    EntityManager(int map_width, int map_height, ProjectArguments& config)
        : player(EntityData(40, 25, '@', 10, color::white), item_info_map),
          map_window(Rect::from_center(player.get_data().x, player.get_data().y,
                                       map_width, map_height)),
          map(map_window.border, config.is_walls_enabled),
          next_worker(create_worker_data()),
          instr_action_clock(0) {

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

    EntityManager(Unpacker& p) : player(p, item_info_map), map_window(p), map(p), next_worker(create_worker_data()) {
        SPDLOG_DEBUG("Unpacking EntityManager");
        ant_proto::EntityManager msg;
        p >> msg;
        instr_action_clock = msg.instr_action_clock();

        ulong worker_count = msg.worker_count();
        SPDLOG_DEBUG("Unpacking workers - count: {}", worker_count);
        for(ulong i = 0; i < worker_count; ++i) {
            ant_proto::Integer worker_msg;
            p >> worker_msg;
            MapEntityType entity_type =
                static_cast<MapEntityType>(worker_msg.value());

            if(entity_type == PLAYER) {
                SPDLOG_WARN("Unexpected player in Worker ant array");
                continue;
            }

            if(entity_type == WORKER) {
                save_ant(new Worker(p, instr_action_clock, item_info_map));
                continue;
            }
            SPDLOG_ERROR("Unknown serialized ant type: {}",
                         static_cast<uint>(entity_type));
            exit(1);
        }

        ulong building_count = msg.building_count();
        SPDLOG_DEBUG("Unpacking building - count: {}", building_count);
        for(ulong i = 0; i < building_count; ++i) {
            ant_proto::Integer building_type_msg;
            p >> building_type_msg;
            BuildingType building_type =
                static_cast<BuildingType>(building_type_msg.value());

            if(building_type == NURSERY) {
                Building* building = new Nursery(p);
                buildings.push_back(building);
                map.add_building(*building);
                continue;
            }

            SPDLOG_ERROR("Unknown serialized building type: {}",
                         static_cast<uint>(building_type));
            exit(1);
        }
    }

    ~EntityManager() {
        SPDLOG_DEBUG("Destroying EntityManager");
        for(auto ant : workers) delete ant;
        for(auto building : buildings) delete building;

        delete next_worker;
        next_worker = nullptr;

        SPDLOG_TRACE("Destroyed workers and buildings");
    }

    void update_fov_ant(EntityData& d) {
        map_window.compute_fov(d.x, d.y, d.fov_radius);
        for(long x = map_window.border.x1; x < map_window.border.x2; x++) {
            for(long y = map_window.border.y1; y < map_window.border.y2;
                y++) {
                if(!map_window.in_fov(x, y)) continue;

                map.explore(x, y);
            }
        }
    }

    void update_fov() {
        // SPDLOG_TRACE("Updating FOV");
        // map.reset_fov();
        for(long x = map_window.border.x1; x < map_window.border.x2; x++) {
            for(long y = map_window.border.y1; y < map_window.border.y2; y++) {
                map.reset_tile(x, y);
            }
        }
        for(auto ant : workers) {
            update_fov_ant(ant->get_data());
        }
        update_fov_ant(player.get_data());
        SPDLOG_TRACE("FOV updated");
    }

    void set_window_tiles() {
        SPDLOG_TRACE("Setting window tiles");
        for(long local_x = 0; local_x < map_window.border.w; local_x++) {
            for(long local_y = 0; local_y < map_window.border.h; local_y++) {
                long x = local_x + map_window.border.x1,
                     y = local_y + map_window.border.y1;
                // SPDLOG_TRACE("Setting tile at ({}, {}) - local ({}, {})", x,
                // y, local_x, local_y);

                if(map.is_wall(x, y)) {
                    map_window.set_wall(x, y);
                } else {
                    map_window.set_floor(x, y);
                }
            }
        }
    }

    void update() {
        ++instr_action_clock;
        if(!map.needs_update) return;
        SPDLOG_TRACE("Updating EntityManager");
        map.needs_update = false;

        map_window.set_center(player.get_data().x, player.get_data().y);
        map.update_chunks(map_window.border);
        set_window_tiles();
        update_fov();
    }

    void create_ant(HardwareManager& hardware_manager,
                    SoftwareManager& software_manager) {
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

        if(!software_manager.has_code()) {
            SPDLOG_ERROR("Cannot create ant with an empty program");
            return;
        }

        Building* b = map.get_building(player);
        if(b == nullptr) {
            SPDLOG_DEBUG("No building found at player location");
            return;
        }

        long new_x = b->border.center_x, new_y = b->border.center_y;
        if(!map.can_place(new_x, new_y)) return;

        SPDLOG_DEBUG("Selected building at ({}, {})", new_x, new_y);

        SPDLOG_DEBUG("Creating worker controller");
        EntityData& data = next_worker->get_data();
        data.x = new_x;
        data.y = new_y;
    
        MachineCode const& code = software_manager.get();

        if (!build_ant(hardware_manager, *next_worker, code)) {
            return;
        }

        SPDLOG_DEBUG("Creating worker ant");
        ulong ant_idx = workers.size();
        software_manager.assign(ant_idx); // before pushing to this->workers
    
        save_ant(next_worker);
        next_worker = create_worker_data();
    }

    bool build_ant(HardwareManager& hardware_manager, Worker& worker, MachineCode const& code) {
        SPDLOG_TRACE("Building ant program - x: {} y: {} - code: {} bytes", worker.get_data().x, worker.get_data().y, code.size());
        AntInteractor interactor(worker.cpu, worker, map,
                            worker.program_executor._ops,
                            worker.program_executor.op_idx, worker.move_speed);
        
        Status status;
        hardware_manager.compile(code, interactor, status);
        if(status.p_err) {
            SPDLOG_ERROR("Failed to compile the program for the ant");
            return false;
        }
        hardware_manager.push_back(&worker.program_executor);
        SPDLOG_TRACE("Successfully compiled worker program");
        return true;
    }

    void rebuild_workers(HardwareManager& hardware_manager, SoftwareManager& software_manager) {
        SPDLOG_DEBUG("Rebuilding worker ant programs - count: {}", workers.size());
        ulong ant_idx = 0;
        for (Worker* worker: workers) {
            build_ant(hardware_manager, *worker, software_manager[ant_idx]); 
            ++ant_idx;
        }
        SPDLOG_TRACE("Completed rebuilding worker ant programs");
    }

    void save_ant(Worker* worker) {
        map.add_entity(*worker);
        workers.push_back(worker);
    }

    Worker* create_worker_data() {
        return new Worker(EntityData('w', 10, color::light_green), instr_action_clock, item_info_map);
    }

    friend Packer& operator<<(Packer& p, EntityManager const& obj) {
        ant_proto::EntityManager msg;
        msg.set_instr_action_clock(obj.instr_action_clock);
        msg.set_worker_count(obj.workers.size());
        msg.set_building_count(obj.buildings.size());
        p << obj.player << obj.map_window << obj.map << msg;
        SPDLOG_DEBUG("Packing entity manager - workers count: {} building count: {}", obj.workers.size(), obj.buildings.size());

        SPDLOG_DEBUG("Packing workers");
        for (Worker const* worker: obj.workers) {
            ant_proto::Integer worker_msg;
            worker_msg.set_value(static_cast<int>(worker->get_type()));
            p << worker_msg << (*worker);
        }

        SPDLOG_DEBUG("Packing buildings");
        for (Building const* building: obj.buildings) {
            ant_proto::Integer building_type_msg;
            building_type_msg.set_value(static_cast<int>(building->get_type()));
            p << building_type_msg << (*building);
        }
        return p;
    }
};
