#include "entity_manager.hpp"
#include "spdlog/spdlog.h"
#include "ui/colors.hpp"

EntityManager::EntityManager(int map_width, int map_height, ProjectArguments& config, ThreadPool<AsyncProgramJob>& job_pool)
    : player(EntityData(40, 25, '@', 10, color::white), item_info_map),
      current_depth(0),
      player_depth(0),
      job_pool(job_pool),
      map_window(Rect::from_center(player.get_data().x, player.get_data().y,
                                   map_width, map_height)),
      map_manager(current_depth, map_width, map_height, config, map_window),
      next_worker(create_worker_data()),
      instr_action_clock(0)
{
    // initialize workers and buildings for ground level
    levels.push_back({{},{}});

    int center_x = map_manager.get_first_room().center_x;
    int center_y = map_manager.get_first_room().center_y;
    levels[current_depth].buildings.push_back(new Nursery(center_x - 1, center_y - 1, 0));
    SPDLOG_INFO("Moving player to nursery building: ({}, {})", center_x,
                center_y);
    player.data.x = center_x, player.data.y = center_y;
    map_manager.get_map().add_building(*levels[current_depth].buildings[0]);

    SPDLOG_DEBUG("Adding player entity to the map");
    map_manager.get_map().add_entity(player);
    map_window.set_center(player.get_data().x, player.get_data().y);
}

EntityManager::EntityManager( ant_proto::EntityManager msg, ThreadPool<AsyncProgramJob>& job_pool) : 
    player(msg.player(), item_info_map), job_pool(job_pool), map_window(msg.map_window()),
    map_manager(msg.map_manager(), current_depth), next_worker(create_worker_data()) 
{
    instr_action_clock = msg.instr_action_clock();
    ulong max_depth = msg.max_depth();
    player_depth = msg.player_depth();

    SPDLOG_DEBUG("Unpacking EntityManager, max_depth {}, player_depth {}", max_depth, player_depth);

    SPDLOG_TRACE("Initializing workers and buildings for unpacking");

    for( current_depth = 0; current_depth < static_cast<unsigned long>(msg.levels_size()); ++current_depth ) {
        levels.push_back({{},{}});
        const ant_proto::Level & level_msg = msg.levels(current_depth);

        for( const auto& worker_msg: level_msg.workers() ) 
            save_ant(new Worker(worker_msg, instr_action_clock, item_info_map, job_pool));

        for( const auto& building_msg: level_msg.buildings() )  {
            Building* building = nullptr;

            if( building_msg.id() == NURSERY)
                building = new Nursery(building_msg);
            else {
                SPDLOG_ERROR("Unknown serialized building type: {}",
                             static_cast<uint>(building_msg.id()));
                exit(1);
            }

            levels[current_depth].buildings.emplace_back(building);
            map_manager.get_map().add_building(*building);

        }

    }

    current_depth = msg.current_depth();

    SPDLOG_TRACE("Unpack of EntityManager finished");
}

EntityManager::~EntityManager() {
    SPDLOG_DEBUG("Destroying EntityManager");
    for( auto level: levels ) {
        for( auto worker: level.workers ) delete worker;
        for( auto building: level.buildings ) delete building;
    }

    delete next_worker;
    next_worker = nullptr;

    SPDLOG_TRACE("Destroyed workers and buildings");
}
// return true of successfully goes up
bool EntityManager::go_up() {
    SPDLOG_TRACE("MAP LEVEL GO UP REQUST");
    if (current_depth == 0) return false;
    --current_depth;
    map_manager.go_up();
    map_manager.get_map().update_chunks(map_window.border);
    return true;
}

// return true of successfully goes down
bool EntityManager::go_down() {
    SPDLOG_TRACE("MAP LEVEL GO DOWN REQUST");
    ++current_depth;
    map_manager.go_down();
    map_manager.get_map().update_chunks(map_window.border);

    if (current_depth == levels.size()) {
        levels.push_back({{},{}});
    }

    return true;
}

std::vector<Building*> EntityManager::get_current_level_buildings() {
    return levels[current_depth].buildings;
}

std::vector<Worker*> EntityManager::get_current_level_worker_ants() {
    return levels[current_depth].workers;
}

void EntityManager::update_fov_ant(EntityData& d) {
    map_window.compute_fov(d.x, d.y, d.fov_radius);
    for(long x = map_window.border.x1; x < map_window.border.x2; x++) {
        for(long y = map_window.border.y1; y < map_window.border.y2;
            y++) {
            if(!map_window.in_fov(x, y)) continue;

            map_manager.get_map().explore(x, y);
        }
    }
}

void EntityManager::update_fov() {
    // SPDLOG_TRACE("Updating FOV");
    // map.reset_fov();
    for(long x = map_window.border.x1; x < map_window.border.x2; x++) {
        for(long y = map_window.border.y1; y < map_window.border.y2; y++) {
            map_manager.get_map().reset_tile(x, y);
        }
    }
    for(auto ant : levels[current_depth].workers) {
        update_fov_ant(ant->get_data());
    }
    if (current_depth == player_depth)
        update_fov_ant(player.get_data());
    SPDLOG_TRACE("FOV updated");
}

void EntityManager::set_window_tiles() {
    SPDLOG_TRACE("Setting window tiles");
    for(long local_x = 0; local_x < map_window.border.w; local_x++) {
        for(long local_y = 0; local_y < map_window.border.h; local_y++) {
            long x = local_x + map_window.border.x1,
                 y = local_y + map_window.border.y1;
            // SPDLOG_TRACE("Setting tile at ({}, {}) - local ({}, {})", x,
            // y, local_x, local_y);

            if(map_manager.get_map().is_wall(x, y)) {
                map_window.set_wall(x, y);
            } else {
                map_window.set_floor(x, y);
            }
        }
    }
}

void EntityManager::update() {
    ++instr_action_clock;
    // Move / dig the ants on the map
    for (size_t depth = 0; depth < levels.size(); ++depth) {
        for (Worker* worker: levels[depth].workers) {
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
                cpu.instr_failed_flag = !map_manager.get_map(depth).move_entity(*worker, dx, dy);
            }
            if (cpu.is_dig_flag) {
                cpu.is_dig_flag = false;
                SPDLOG_DEBUG("Digging worker - dx: {} dy: {}", dx, dy);
                cpu.instr_failed_flag = !map_manager.get_map(depth).dig(*worker, dx, dy);
            }
            if (cpu.delta_scents) {
                ulong& tile_scents = map_manager.get_map(depth).get_tile_scents(*worker);

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

    if(!map_manager.get_map().needs_update) return;
    SPDLOG_TRACE("Updating EntityManager");
    map_manager.get_map().needs_update = false;

    map_window.set_center(player.get_data().x, player.get_data().y);
    map_manager.get_map().update_chunks(map_window.border);
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

    Building* b = map_manager.get_map().get_building(player);
    if(b == nullptr) {
        SPDLOG_DEBUG("No building found at player location");
        return;
    }

    long new_x = b->border.center_x, new_y = b->border.center_y;
    if(!map_manager.get_map().can_place(new_x, new_y)) return;

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
    ulong ant_idx = levels[current_depth].workers.size();
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
    SPDLOG_DEBUG("Rebuilding worker ant programs - count: {}", levels.size());
    ulong ant_idx = 0;
    for(auto& level: levels) {
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
    for(const auto& level: levels) {
        n += level.workers.size();
    }
    return n;
}

void EntityManager::save_ant(Worker* worker) {
    map_manager.get_map().add_entity(*worker);
    levels[current_depth].workers.push_back(worker);
}

Worker* EntityManager::create_worker_data() {
    return new Worker(EntityData('w', 10, color::light_green),
            instr_action_clock, item_info_map, job_pool);
}

ant_proto::EntityManager EntityManager::get_proto() const {
    ant_proto::EntityManager entity_msg;
    entity_msg.set_instr_action_clock(instr_action_clock);
    entity_msg.set_max_depth(levels.size());
    entity_msg.set_current_depth(current_depth);
    entity_msg.set_player_depth(player_depth);
    *entity_msg.mutable_player() = player.get_proto();
    for( const auto& level: levels ) *entity_msg.add_levels() = level.get_proto();
    *entity_msg.mutable_map_window() = map_window.get_proto();
    *entity_msg.mutable_map_manager() = map_manager.get_proto();
    

    return entity_msg;
}

