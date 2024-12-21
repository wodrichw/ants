#pragma once

#include "entity.pb.h"
#include "entity/ant.hpp"
#include "entity/entity_data.hpp"
#include "hardware/hardware_manager.hpp"
#include "hardware/program_executor.hpp"
#include "hardware/software_manager.hpp"
#include "map/manager.hpp"
#include "map/world.hpp"
#include "utils/thread_pool.hpp"

struct EntityManager {
    MapManager& map_manager;
    MapWorld& map_world;
    Player player;
    ulong player_depth;
    ThreadPool<AsyncProgramJob>& job_pool;
    Worker* next_worker = nullptr;

    EntityManager(MapManager& map_manager, MapWorld& map_world,
                  int player_start_x, int player_start_y,
                  ThreadPool<AsyncProgramJob>& job_pool);
    EntityManager(ant_proto::EntityManager msg, MapManager& map_manager,
                  MapWorld& map_world, ThreadPool<AsyncProgramJob>& job_pool);
    ~EntityManager();

    void update_fov();
    void update();
    void create_ant(HardwareManager& hardware_manager,
                    SoftwareManager& software_manager);
    bool build_ant(HardwareManager& hardware_manager, Worker& worker,
                   MachineCode const& machine_code);
    void rebuild_workers(HardwareManager& hardware_manager,
                         SoftwareManager& software_manager);
    ulong
    num_workers();  // returns the total number of workers accross all levels
    void save_ant(Worker* worker);
    Worker* create_worker_data();
    ant_proto::EntityManager get_proto() const;
};
