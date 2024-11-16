#include "entity/entity_manager.hpp"
#include "map/world.hpp"
#include "spdlog/spdlog.h"

EntityManager::EntityManager(MapWorld& map_world, int player_start_x, int player_start_y, ThreadPool<AsyncProgramJob>& job_pool): 
    map_world(map_world),
    player(EntityData(40, 25, '@', 10, color::white), map_world.item_info_map),
    player_depth(0),
    job_pool(job_pool),
    next_worker(create_worker_data())
{
    player.data.x = player_start_x;
    player.data.y = player_start_y;
}

EntityManager::EntityManager( ant_proto::EntityManager msg, MapWorld& map_world, ThreadPool<AsyncProgramJob>& job_pool) : 
    map_world(map_world),
    player(msg.player(), map_world.item_info_map),
    player_depth(msg.player_depth()),
    job_pool(job_pool),
    next_worker(create_worker_data())
{ }

EntityManager::~EntityManager() {
    SPDLOG_DEBUG("Destroying EntityManager");
    delete next_worker;
    next_worker = nullptr;

    SPDLOG_TRACE("Destroyed workers and buildings");
}

void EntityManager::update_fov_ant(EntityData& d) {
    map_world.map_window.compute_fov(d.x, d.y, d.fov_radius);
    for(long x = map_world.map_window.border.x1; x < map_world.map_window.border.x2; x++) {
        for(long y = map_world.map_window.border.y1; y < map_world.map_window.border.y2;
            y++) {
            if(!map_world.map_window.in_fov(x, y)) continue;

            map_world.current_level().map.explore(x, y);
        }
    }
}

void EntityManager::update_fov() {
    // SPDLOG_TRACE("Updating FOV");
    // map.reset_fov();
    for(long x = map_world.map_window.border.x1; x < map_world.map_window.border.x2; x++) {
        for(long y = map_world.map_window.border.y1; y < map_world.map_window.border.y2; y++) {
            map_world.current_level().map.reset_tile(x, y);
        }
    }
    for(auto ant : map_world.current_level().workers) {
        update_fov_ant(ant->get_data());
    }
    if (map_world.current_depth == player_depth)
        update_fov_ant(player.get_data());
    SPDLOG_TRACE("FOV updated");
}

void EntityManager::set_window_tiles() {
    SPDLOG_TRACE("Setting window tiles");
    for(long local_x = 0; local_x < map_world.map_window.border.w; local_x++) {
        for(long local_y = 0; local_y < map_world.map_window.border.h; local_y++) {
            long x = local_x + map_world.map_window.border.x1,
                 y = local_y + map_world.map_window.border.y1;
            // SPDLOG_TRACE("Setting tile at ({}, {}) - local ({}, {})", x,
            // y, local_x, local_y);

            if(map_world.current_level().map.is_wall(x, y)) {
                map_world.map_window.set_wall(x, y);
            } else {
                map_world.map_window.set_floor(x, y);
            }
        }
    }
}

void EntityManager::update() {
    ++map_world.instr_action_clock;
    // Move / dig the ants on the map
    for (auto& level: map_world.levels) {
        for (Worker* worker: level.workers) {
            DualRegisters& cpu = worker->cpu;

            // Direction Truth Table
            // A B | DX DY
            // 0 0 |  1  0
            // 0 1 |  0 -1
            // 1 0 | -1  0
            // 1 1 |  0  1

            long dx = (1 - cpu.dir_flag2) * (-2 * cpu.dir_flag1 + 1);
            long dy = cpu.dir_flag2 * (2 * cpu.dir_flag1 - 1);
            if (cpu.is_move_flag) {
                cpu.is_move_flag = false;
                SPDLOG_DEBUG("Moving worker - dx: {} dy: {}", dx, dy);
                cpu.instr_failed_flag = !level.map.move_entity(*worker, dx, dy);
            }
            if (cpu.is_dig_flag) {
                cpu.is_dig_flag = false;
                SPDLOG_DEBUG("Digging worker - dx: {} dy: {}", dx, dy);
                cpu.instr_failed_flag = !level.map.dig(*worker, dx, dy);
            }
            if (cpu.delta_scents) {
                ulong& tile_scents = level.map.get_tile_scents(*worker);

                ulong updated_scents = 0;
                ulong offset = 0;
                while (cpu.delta_scents != 0) {
                    ulong delta_scent = cpu.delta_scents & 0xFF;
                    ulong prev_scent = tile_scents & 0xFF;
                    ulong scent = (prev_scent + delta_scent) & 0xFF;
                    updated_scents |= (scent << offset);

                    tile_scents >>= 8;
                    cpu.delta_scents >>= 8;
                    offset += 8;
                }
                tile_scents = updated_scents;
                // SPDLOG_INFO("Tile scent: {} - x: {} y: {}", tile_scents, worker->get_data().x, worker->get_data().y);
            }
        }
    }

    if(!map_world.current_level().map.needs_update) return;
    SPDLOG_TRACE("Updating EntityManager");
    map_world.current_level().map.needs_update = false;

    map_world.map_window.set_center(player.get_data().x, player.get_data().y);
    map_world.current_level().map.update_chunks(map_world.map_window.border);
    set_window_tiles();
    update_fov();
}

void EntityManager::create_ant(HardwareManager& hardware_manager,
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

    Building* b = map_world.current_level().map.get_building(player);
    if(b == nullptr) {
        SPDLOG_DEBUG("No building found at player location");
        return;
    }

    long new_x = b->border.center_x, new_y = b->border.center_y;
    if(!map_world.current_level().map.can_place(new_x, new_y)) return;

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
    ulong ant_idx = map_world.current_level().workers.size();
    software_manager.assign(ant_idx); // before pushing to this->workers

    save_ant(next_worker);
    next_worker = create_worker_data();
}

bool EntityManager::build_ant(HardwareManager& hardware_manager,
        Worker& worker,
        MachineCode const& machine_code)
{
    SPDLOG_TRACE("Building ant program - x: {} y: {} - machine_code: {} bytes", worker.get_data().x, worker.get_data().y, machine_code.size());

    CompileArgs compile_args(machine_code.code, worker.cpu, worker.program_executor._ops);
    hardware_manager.compile(compile_args);
    if(compile_args.status.p_err) {
        SPDLOG_ERROR("Failed to compile the program for the ant");
        return false;
    }
    hardware_manager.push_back(&worker.program_executor);
    SPDLOG_TRACE("Successfully compiled worker program");
    return true;
}

void EntityManager::rebuild_workers(HardwareManager& hardware_manager,
        SoftwareManager& software_manager)
{
    SPDLOG_DEBUG("Rebuilding worker ant programs - count: {}", map_world.levels.size());
    ulong ant_idx = 0;
    for(auto& level: map_world.levels) {
        for (Worker* worker: level.workers) {
            build_ant(hardware_manager, *worker, software_manager[ant_idx]);
            ++ant_idx;
        }
    }
    SPDLOG_TRACE("Completed rebuilding worker ant programs");
}

//returns the total number of workers accross all levels
ulong EntityManager::num_workers() {
    ulong n = 0;
    for(const auto& level: map_world.levels) {
        n += level.workers.size();
    }
    return n;
}

void EntityManager::save_ant(Worker* worker) {
    map_world.current_level().map.add_entity(*worker);
    map_world.current_level().workers.push_back(worker);
}

Worker* EntityManager::create_worker_data() {
    return new Worker(EntityData('w', 10, color::light_green),
            map_world.instr_action_clock, map_world.item_info_map, job_pool);
}

ant_proto::EntityManager EntityManager::get_proto() const {
    ant_proto::EntityManager entity_msg;
    entity_msg.set_player_depth(player_depth);
    *entity_msg.mutable_player() = player.get_proto();

    return entity_msg;
}

