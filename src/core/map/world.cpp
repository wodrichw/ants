#include "map.pb.h"
#include "entity/inventory.hpp"
#include "entity/ant.hpp"
#include "map/world.hpp"
#include "map/window.hpp"

Level::Level(const Map& map):
    map(map)
{}


Level::Level(const ant_proto::Level& msg, const ulong &instr_clock, const ItemInfoMap& item_map, ThreadPool<AsyncProgramJob>& thread_pool,  bool is_walls_enabled):
    workers{},
    buildings{},
    map(msg.map(), is_walls_enabled)
{
    for(const auto& worker_msg: msg.workers())
        workers.emplace_back(new Worker(worker_msg, instr_clock, item_map, thread_pool));

    for(const auto& building_msg: msg.buildings()) add_building(building_msg);
}

ant_proto::Level Level::get_proto() const {
    ant_proto::Level msg;
    for( const auto worker: workers ) *msg.add_workers() = worker->get_proto();
    for( const auto building: buildings ) *msg.add_buildings() = building->get_proto();
    *msg.mutable_map() = map.get_proto();
    
    return msg;
}


void Level::add_building(const Building& building) {
    if( building.id == NURSERY ) {
        buildings.emplace_back(new Nursery(building));
        map.add_building(*buildings.back());
    } else {
        SPDLOG_ERROR("Building type: {} is unknown", building.id);
        return;
    }
}

void Level::add_building(const ant_proto::Building& building) {
    if( building.id() == NURSERY ) {
        buildings.emplace_back(new Nursery(building));
        map.add_building(*buildings.back());
    } else {
        SPDLOG_ERROR("Building type: {} is unknown", building.id());
        return;
    }
}

MapWorld::MapWorld(const Rect& border): 
    levels{},
    map_window(border)
{}

MapWorld::MapWorld(const ant_proto::MapWorld& msg, ThreadPool<AsyncProgramJob>& thread_pool,  bool is_walls_enabled):
    levels{},
    map_window(msg.map_window()),
    current_depth(msg.current_depth()),
    item_info_map(),
    instr_action_clock(msg.instr_action_clock())
{
    for(const auto& level_msg: msg.levels())
        levels.emplace_back(Level(level_msg, instr_action_clock, item_info_map, thread_pool, is_walls_enabled));
}


Level& MapWorld::current_level() {
   return levels[current_depth]; 
}

Level& MapWorld::operator[](ulong depth) {
    return levels[depth];
}


ant_proto::MapWorld MapWorld::get_proto() const {
    ant_proto::MapWorld msg;

    for(const auto& level: levels) *msg.add_levels() = level.get_proto();
    msg.set_current_depth(current_depth);
    *msg.mutable_map_window() = map_window.get_proto();

    return msg;
}
