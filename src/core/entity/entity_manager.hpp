#pragma once

#include <vector>

#include "app/arg_parse.hpp"
#include "entity.pb.h"
#include "entity/ant.hpp"
#include "entity/building.hpp"
#include "entity/entity_data.hpp"
#include "entity/map_manager.hpp"
#include "entity/map_window.hpp"
#include "hardware/hardware_manager.hpp"
#include "hardware/program_executor.hpp"
#include "hardware/software_manager.hpp"
#include "utils/thread_pool.hpp"

struct Level {
    std::vector<Worker*> workers = {};
    std::vector<Building*> buildings = {};

    ant_proto::Level get_proto() const {
        ant_proto::Level msg;
        for( auto& worker: workers )
            *msg.add_workers() = worker->get_proto();
        for( auto& building: buildings )
            *msg.add_buildings() = building->get_proto();

        return msg;
    }
};

struct EntityManager {
    ItemInfoMap item_info_map = {};
    Player player;
    ulong current_depth;
    ulong player_depth;
    ThreadPool<AsyncProgramJob>& job_pool;
    std::vector<Level> levels = {};
    MapWindow map_window;
    MapManager map_manager;
    Worker* next_worker = nullptr;
    ulong instr_action_clock = 0;


    EntityManager(int map_width, int map_height, ProjectArguments& config, ThreadPool<AsyncProgramJob>& job_pool);
    EntityManager( ant_proto::EntityManager msg, ThreadPool<AsyncProgramJob>& job_pool);
    ~EntityManager();

    bool go_up();   // return true of successfully goes up
    bool go_down(); // return true of successfully goes down
                    //
    std::vector<Building*> get_current_level_buildings();
    std::vector<Worker*> get_current_level_worker_ants();

    void update_fov_ant(EntityData& d);
    void update_fov();
    void set_window_tiles();
    void update();
    void create_ant(HardwareManager& hardware_manager, SoftwareManager& software_manager);
    bool build_ant(HardwareManager& hardware_manager, Worker& worker, MachineCode const& machine_code);
    void rebuild_workers(HardwareManager& hardware_manager, SoftwareManager& software_manager);
    ulong num_workers(); //returns the total number of workers accross all levels
    void save_ant(Worker* worker);
    Worker* create_worker_data();
    ant_proto::EntityManager get_proto() const;
};
