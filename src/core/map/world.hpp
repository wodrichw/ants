#pragma once

#include "entity.pb.h"
#include "map.pb.h"
#include "map/map.hpp"
#include "map/window.hpp"
#include "entity/inventory.hpp"
#include "utils/thread_pool.hpp"
#include "hardware/program_executor.hpp"

class Worker;
struct Building;
class ItemInfoMap;

struct Level {
    std::vector<Worker*> workers = {};
    std::vector<Building*> buildings = {};
    Map map;

    Level(const Map& map);
    Level(const ant_proto::Level& msg, const ulong &instr_clock, const ItemInfoMap& item_map, ThreadPool<AsyncProgramJob>& thread_pool,  bool is_walls_enabled);

    ant_proto::Level get_proto() const;

    void add_building(const Building& building);
    void add_building(const ant_proto::Building& building);
};

class MapWorld {
public:
    std::vector<Level> levels;
    MapWindow map_window;
    ulong current_depth;
    ItemInfoMap item_info_map = {};
    ulong instr_action_clock = 0;

    MapWorld(const Rect& border);
    MapWorld(const ant_proto::MapWorld& msg, ThreadPool<AsyncProgramJob>& thread_pool,  bool is_walls_enabled);
    ant_proto::MapWorld get_proto() const;

    Level& current_level();
    Level& operator[](ulong depth);
};

