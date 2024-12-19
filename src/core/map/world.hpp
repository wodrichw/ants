#pragma once

#include <libtcod/mersenne.hpp>
#include <optional>
#include "app/globals.hpp"
#include "entity.pb.h"
#include "map.pb.h"
#include "map/map.hpp"
#include "map/window.hpp"
#include "entity/inventory.hpp"
#include "utils/math.hpp"
#include "utils/thread_pool.hpp"
#include "hardware/program_executor.hpp"

class Worker;
struct Building;
class ItemInfoMap;
struct Level;


struct Section_Plan {
    using buildf_t = std::function<void(Level&, Section_Plan&)>;
    Rect border;
    ulong depth_in_zone;
    bool in_construction;
    Section_Plan(const Rect& border, ulong depth_in_zone, buildf_t _build_section) :
        border(border), depth_in_zone(depth_in_zone), in_construction(false), _build_section(_build_section) {}
    buildf_t _build_section;
    void build_section(Level& l) { _build_section(l, *this); }
};


struct Start_Data {
    long player_x;
    long player_y;
};


struct Level {
    using f_xy_t = std::function<void(long, long)>;
    std::optional<Start_Data> start_info = {}; // only filled if required for start position
    std::vector<Worker*> workers = {};
    std::vector<Building*> buildings = {};
    Map map;
    ulong depth;

    Level(const Map& map, ulong depth);
    Level(const ant_proto::Level& msg,
        const ulong &instr_clock,
        const ItemInfoMap& item_map,
        ThreadPool<AsyncProgramJob>& thread_pool,
        bool is_walls_enabled,
        f_xy_t pre_chunk_generation_callback
    );

    ant_proto::Level get_proto() const;

    void add_building(const Building& building);
    void add_building(const ant_proto::Building& building);
};


struct Zone {
    ulong w;
    ulong h;
    ulong depth;
    std::function<void(Level&,Section_Plan&)> build_section;
    Zone(ulong w, ulong h, ulong depth, std::function<void(Level&,Section_Plan&)> build_section);
    
    virtual ~Zone() {}
};


struct Peaceful_Cavern : public Zone{
    Peaceful_Cavern();
    ~Peaceful_Cavern() {}
    static void build_section(Level&, Section_Plan&);
};


struct Starting_Colony : public Zone{
    Starting_Colony();
    ~Starting_Colony() {}
    static void build_section(Level&, Section_Plan&);

};


struct Region {
    using chunk_assignments_t = std::vector<std::vector<std::vector<Zone*>>>;
    uint32_t seed_x;
    uint32_t seed_y;
    Rect perimeter;
    TCODRandom randomizer;
    bool is_first_region;
    std::vector<std::vector<Section_Plan>> section_plans = {};

    Region() = delete;
    Region(const Rect& perimeter);
    Region(uint32_t seed_x, uint32_t seed_y, const Rect& perimeter);
    Region(const ant_proto::Region& msg);
    ant_proto::Region get_proto() const;

    Section_Plan* section_plan( long x, long y, long z );
    uint32_t get_seed();
    bool can_place_zone(chunk_assignments_t& chunk_assignemnts, long x, long y, long z, Zone& zone);
    void place_zone(chunk_assignments_t& chunk_assignemnts, long x, long y, long z, Zone& zone);
    void do_blueprint_planning();
};


struct Region_Key {
    long x;
    long y;
    bool operator==( const Region_Key& rhs ) {
        return div_floor(x, globals::WORLD_LENGTH) < div_floor(rhs.x, globals::WORLD_LENGTH) &&
            div_floor(y, globals::WORLD_LENGTH) < div_floor(rhs.y, globals::WORLD_LENGTH);
    }
};


namespace std {
    template <>
    struct hash<Region_Key> {
        size_t operator()(const Region_Key& k) const {
            return std::hash<long>()(div_floor(k.x, globals::WORLD_LENGTH)) ^
                std::hash<long>()(div_floor(k.y, globals::WORLD_LENGTH));
        }
    };

    template<>
    struct equal_to<Region_Key> {
        bool operator()(const Region_Key& lhs, const Region_Key& rhs) const {
            return div_floor(lhs.x, globals::WORLD_LENGTH) == div_floor(rhs.x, globals::WORLD_LENGTH) &&
                div_floor(lhs.y, globals::WORLD_LENGTH) == div_floor(rhs.y, globals::WORLD_LENGTH);
        }

    };
}


struct Regions {
    std::unordered_map<Region_Key, Region> rmap;

    long align_to_region(long pos);
    void build_section(long x, long y, Level& l);

    Regions();
    Regions(const ant_proto::Regions msg);
    ant_proto::Regions get_proto() const;
};


class MapWorld {
public:
    std::vector<Level> levels;
    Regions regions;
    MapWindow map_window;
    ulong current_depth;
    ItemInfoMap item_info_map = {};
    ulong instr_action_clock = 0;

    MapWorld(const Rect& border, bool is_walls_enabled); // guaranteed first world
    MapWorld(const ant_proto::MapWorld& msg, ThreadPool<AsyncProgramJob>& thread_pool,  bool is_walls_enabled);
    ant_proto::MapWorld get_proto() const;

    Level& current_level();
    Level& operator[](ulong depth);
};

