#include "app/globals.hpp"
#include "map.pb.h"
#include "entity/inventory.hpp"
#include "entity/ant.hpp"
#include "map/world.hpp"
#include <algorithm>
#include <climits>
#include <cstdint>
#include <libtcod/mersenne.hpp>
#include "map/builder.hpp"
#include "map/section_data.hpp"
#include "map/window.hpp"
#include "utils/algorithm.hpp"

Level::Level(const Map& map):
    map(map)
{}


Level::Level(
    const ant_proto::Level& msg,
    const ulong &instr_clock,
    const ItemInfoMap& item_map,
    ThreadPool<AsyncProgramJob>& thread_pool,
    bool is_walls_enabled
):
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


Zone::Zone(ulong w, ulong h, ulong depth, std::function<void(Level&,Section_Plan&)> build_section):
    w(w), h(h), depth(depth), build_section(build_section)
{}


Peaceful_Cavern::Peaceful_Cavern():
    Zone(
        globals::MIN_SECTION_LENGTH*2,
        globals::MIN_SECTION_LENGTH*2,
        globals::MAX_LEVEL_DEPTH,
        build_section
    )
{}


void Peaceful_Cavern::build_section(Level& l, Section_Plan& sp) {
    MapSectionData section;
    RandomMapBuilder(Rect(sp.x, sp.y, sp.w, sp.h))(section);
    l.map.load_section(section);
}


Starting_Colony::Starting_Colony():
    Zone{
        globals::MIN_SECTION_LENGTH*2,
        globals::MIN_SECTION_LENGTH*2,
        globals::MAX_LEVEL_DEPTH,
        build_section
    }
{}


void Starting_Colony::build_section(Level& l, Section_Plan& sp) {
    MapSectionData section;
    EmptyMapBuilder(Rect(sp.x,sp.y,sp.w,sp.h));
    l.map.load_section(section);
    if( sp.depth_in_zone == 0 && l.depth == 0){ // we know that zone takes up entire depth
        Rect& first_room = section.rooms[0];
        int center_x = first_room.center_x;
        int center_y = first_room.center_y;
        l.add_building(Nursery(center_x - 1, center_y - 1, 0));
        l.start_info.emplace(Start_Data{center_x, center_y}); // record where player should start
    }
}


class World_BspListener : public ITCODBspCallback {
private:
    Rect perimeter;
    std::vector<Rect>& columns;
public:
    World_BspListener(const Rect& perimeter, std::vector<Rect>& columns) :
        perimeter(perimeter),
        columns(columns)
    {}

    bool visitNode(TCODBsp *node, void *user_data) {
        (void)user_data;  // don't need user_data
        if(!node->isLeaf()) return true;
        columns.emplace_back(Rect(node->x, node->y, node->w, node->h));
        return true;
    }
};


bool Region::can_place_zone(chunk_assignemnts_t& chunk_assignemnts, long x, long y, long z, Zone& zone) {
    for(ulong i = x; i < x + zone.w; ++i) {
        for(ulong j = y; j < y + zone.h; ++j) {
            for(ulong k = z; k < z + zone.depth; ++k) {
                if( chunk_assignemnts[i][j][k] != nullptr ) return false;
            }
        }
    }
    return true;
}


void Region::place_zone(chunk_assignemnts_t& chunk_assignemnts, long x, long y, long z, Zone& zone) {
    for(ulong i = x; i < x + zone.w; ++i) {
        for(ulong j = y; j < y + zone.h; ++j) {
            for(ulong k = z; k < z + zone.depth; ++k) {
                chunk_assignemnts[i][j][k] = &zone;
            }
        }
    }
}


void Region::do_blueprint_planning() {
    // randomly partition world into smaller columns.
    // These columns will then be sliced into
    // section plans to go into the blueprints for each level

    // GENERATE columns
    // std::vector<Rect> columns;

    // TCODBsp bsp(perimeter.x1, perimeter.y1, perimeter.w, perimeter.h);
    // int nb = 8;  // max level of recursion -- can make 2^nb rooms.
    // bsp.splitRecursive(&randomizer, nb, globals::MIN_SECTION_LENGTH, globals::MIN_SECTION_LENGTH, 1, 1);
    // World_BspListener listener(perimeter, columns);
    // bsp.traverseInvertedLevelOrder(&listener, NULL);
    
    long xy_length = globals::WORLD_LENGTH/globals::CHUNK_LENGTH;
    chunk_assignemnts_t chunk_assignemnts(xy_length, std::vector<std::vector<Zone*>>(xy_length, std::vector<Zone*>(globals::MAX_LEVEL_DEPTH, 0)));

    struct placed_zones_t {long chunk_x; long chunk_y; long chunk_z; Zone* zone;};
    std::vector<placed_zones_t> placed_zones;

    Zone* starting_coloney = new Starting_Colony();
    place_zone(chunk_assignemnts, 0,0,0, *starting_coloney);
    placed_zones.emplace_back(placed_zones_t{0,0,0, starting_coloney});

    std::vector<Zone* > shapes = { new Peaceful_Cavern() };
    std::shuffle(shapes.begin(), shapes.end(), randomizer);

    
    uint failed_attempts = 0;
    while( failed_attempts < 10 ) {
        bool successful_attempt = false;
        for( const auto& shape: shapes) {
            bool placed = false;
            long shape_width = shape->w;
            long shape_height = shape->h;
            for(long z = 0; z <= globals::MAX_LEVEL_DEPTH; ++z ) {
                for(long x = 0; x <= xy_length - shape_width; ++x) {
                    for(long y = 0; y <= xy_length - shape_height; ++y) {
                        if(can_place_zone( chunk_assignemnts, x, y, z, *shape ) ) {
                            place_zone( chunk_assignemnts, x, y, z, *shape );
                            placed_zones.emplace_back(placed_zones_t{x,y,z,shape});
                            placed = true;
                            break;
                        }
                    }
                    if(placed) {
                        break;
                    }
                }
            }
            if(placed) successful_attempt = true;
        }
        if(! successful_attempt ) failed_attempts++;
        std::shuffle(shapes.begin(), shapes.end(), randomizer);
    }
    

    // Plan out the sections in each level
    std::sort(placed_zones.begin(), placed_zones.end(), [](placed_zones_t lhs, placed_zones_t rhs){
            if      (lhs.chunk_x < rhs.chunk_x ) return true;
            else if (lhs.chunk_x > rhs.chunk_x ) return false;
            // x is equal
            else if (lhs.chunk_y < rhs.chunk_y ) return true;
            else if (lhs.chunk_y > rhs.chunk_y ) return false;
            // x, and y are equal
            else if (lhs.chunk_z < rhs.chunk_z ) return true;
            else if (lhs.chunk_z > rhs.chunk_z ) return false;
            // x, y, and z are equal
            else return false;
    });

    for(const auto& zone_placement: placed_zones) {
        for(
            ulong z = zone_placement.chunk_z;
            z < zone_placement.chunk_z + zone_placement.zone->depth;
            ++z
        ){
            section_plans.emplace_back(Section_Plan({
                perimeter.x1 + (zone_placement.chunk_x * globals::CHUNK_LENGTH),
                perimeter.y1 + (zone_placement.chunk_y * globals::CHUNK_LENGTH),
                zone_placement.zone->w * globals::CHUNK_LENGTH,
                zone_placement.zone->h * globals::CHUNK_LENGTH,
                z - zone_placement.chunk_z,
                false,
                zone_placement.zone->build_section
            }));
        }
    }
}


uint32_t Region::get_seed() {
    return seed_x ^ seed_y;
}


Region::Region( const Rect& perimeter): 
    perimeter(perimeter),
    is_first_region(false)
{
    // NO Seeds provided so generate them
    TCODRandom *rng = TCODRandom::getInstance();
    seed_x = rng->getInt(INT_MIN, INT_MAX);
    seed_y = rng->getInt(INT_MIN, INT_MAX);
    randomizer = TCODRandom(get_seed());

    do_blueprint_planning();
}


Region::Region(
    uint32_t seed_x,
    uint32_t seed_y,
    const Rect& perimeter
): 
    seed_x(seed_x),
    seed_y(seed_y),
    perimeter(perimeter),
    randomizer(get_seed()),
    is_first_region(false)
{
    // NO Seeds provided so generate them
    TCODRandom *rng = TCODRandom::getInstance();
    seed_x = rng->getInt(INT_MIN, INT_MAX);
    seed_y = rng->getInt(INT_MIN, INT_MAX);
    randomizer = TCODRandom(get_seed());

    do_blueprint_planning();
}


Region::Region(const ant_proto::Region& msg):
    seed_x(msg.seed_x()),
    seed_y(msg.seed_y()),
    perimeter(msg.perimeter()),
    is_first_region(msg.is_first_region())
{}


ant_proto::Region Region::get_proto() {
    ant_proto::Region msg;
    msg.set_seed_x(seed_x);
    msg.set_seed_y(seed_y);
    return msg;
}


MapWorld::MapWorld(const Rect& border, bool is_walls_enabled): 
    levels{},
    regions(12),
    map_window(border)
{
    // Ensure that levels are created
    for( size_t i = 0; i < globals::MAX_LEVEL_DEPTH; ++i )
        if( levels.size() <= i ) levels.emplace_back(Level(Map(border, is_walls_enabled)));

    // Build first region
    regions.emplace(Region_Key{0,0}, Region(Rect(0,0, globals::WORLD_LENGTH, globals::WORLD_LENGTH)));
}


MapWorld::MapWorld(
    const ant_proto::MapWorld& msg,
    ThreadPool<AsyncProgramJob>& thread_pool,
    bool is_walls_enabled
):
    levels{},
    map_window(msg.map_window()),
    current_depth(msg.current_depth()),
    item_info_map(),
    instr_action_clock(msg.instr_action_clock())
{
    for( int i = 0; i < msg.levels().size(); ++i )
        levels[i] = Level(msg.levels()[i], instr_action_clock, item_info_map, thread_pool, is_walls_enabled);
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

