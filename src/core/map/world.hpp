#pragma once

#include <libtcod/mersenne.hpp>
#include <optional>

#include "app/globals.hpp"
#include "entity.pb.h"
#include "entity/inventory.hpp"
#include "hardware/program_executor.hpp"
#include "map.pb.h"
#include "map/map.hpp"
#include "map/window.hpp"
#include "utils/math.hpp"
#include "utils/thread_pool.hpp"

struct Worker;
struct Building;
class ItemInfoMap;
struct Level;

struct Section_Plan {
    using buildf_t = std::function<void(Level&, Section_Plan&)>;
    Rect border;
    ulong depth_in_zone;
    bool in_construction;
    Section_Plan(const Rect& border, ulong depth_in_zone,
                 buildf_t _build_section)
        : border(border),
          depth_in_zone(depth_in_zone),
          in_construction(false),
          _build_section(_build_section) {}
    buildf_t _build_section;
    void build_section(Level& l) { _build_section(l, *this); }
};

struct Start_Data {
    long player_x;
    long player_y;
};

struct Level {
    using f_xy_t = std::function<void(long, long)>;
    std::optional<Start_Data> start_info =
        {};  // only filled if required for start position
    std::vector<Worker*> workers = {};
    std::vector<Building*> buildings = {};
    Map map;
    ulong depth;

    Level(const Map& map, ulong depth);
    Level(const ant_proto::Level& msg, const ulong& instr_clock,
          const ItemInfoMap& item_map, ThreadPool<AsyncProgramJob>& thread_pool,
          bool is_walls_enabled, f_xy_t pre_chunk_generation_callback);

    ant_proto::Level get_proto() const;

    void add_building(const Building& building);
    void add_building(const ant_proto::Building& building);
};

struct Zone {
    ulong w;
    ulong h;
    ulong depth;
    std::function<void(Level&, Section_Plan&)> build_section;
    Zone(ulong w, ulong h, ulong depth,
         std::function<void(Level&, Section_Plan&)> build_section);

    virtual ~Zone() {}
};

struct Peaceful_Cavern : public Zone {
    Peaceful_Cavern();
    ~Peaceful_Cavern() {}
    static void build_section(Level&, Section_Plan&);
};

struct Starting_Colony : public Zone {
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
    ant_proto::Region get_proto();

    // Section_Plan& section_plan(long x, long y) {
    Section_Plan* section_plan(long x, long y, long z) {
        std::vector<Section_Plan>& level_sp = section_plans[z];

        // auto search_sp = std::lower_bound(level_sp.begin(), level_sp.end(),
        // Section_Plan(Rect(x,y, 1, 1), 0, [](Level&, Section_Plan&){}),
        //     [](const Section_Plan& l, const Section_Plan& r) {
        //         std::cout << "DEBUG: " << l.border.x1 << "  " << l.border.y1
        //         << " : " << r.border.x1 << "  " << r.border.y1; if(
        //         l.border.x1 < r.border.x1 ) { std::cout << " t" << std::endl;
        //         return true; } if( l.border.x1 > r.border.x1 ) { std::cout <<
        //         " f" << std::endl; return false; } if( l.border.y1 <
        //         r.border.y1 ) { std::cout << " t" << std::endl; return true;
        //         } std::cout << " f" << std::endl; return false;
        //     }
        // );
        //
        // if( search_sp == level_sp.end() ) return nullptr;
        // ++search_sp;  // we want the seaerch that likely contains our x,y
        // coordinates
        // // if( search_sp == level_sp.end() ) return nullptr;
        //
        // if(! (x >= search_sp->border.x1 && x <= search_sp->border.x2) )
        // return nullptr; if(! (y >= search_sp->border.y1 && y <=
        // search_sp->border.y2) ) return nullptr; return &*search_sp;

        // linear search for now, if a performance issue then use binary search
        // like ^ but without the bugs
        for(auto& scan_sp : level_sp) {
            if(scan_sp.border.x1 <= x && scan_sp.border.x2 >= x &&
               scan_sp.border.y1 <= y && scan_sp.border.y2 >= y)
                return &scan_sp;
        }
        return nullptr;
    }

    uint32_t get_seed();
    bool can_place_zone(chunk_assignments_t& chunk_assignemnts, long x, long y,
                        long z, Zone& zone);
    void place_zone(chunk_assignments_t& chunk_assignemnts, long x, long y,
                    long z, Zone& zone);
    void do_blueprint_planning();
};

struct Region_Key {
    long x;
    long y;
    bool operator==(const Region_Key& rhs) {
        return div_floor(x, globals::WORLD_LENGTH) <
                   div_floor(rhs.x, globals::WORLD_LENGTH) &&
               div_floor(y, globals::WORLD_LENGTH) <
                   div_floor(rhs.y, globals::WORLD_LENGTH);
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

    template <>
    struct equal_to<Region_Key> {
        bool operator()(const Region_Key& lhs, const Region_Key& rhs) const {
            return div_floor(lhs.x, globals::WORLD_LENGTH) ==
                       div_floor(rhs.x, globals::WORLD_LENGTH) &&
                   div_floor(lhs.y, globals::WORLD_LENGTH) ==
                       div_floor(rhs.y, globals::WORLD_LENGTH);
        }
    };
}  // namespace std

struct Regions {
    long align_to_region(long pos) {
        return div_floor(pos, globals::WORLD_LENGTH) * globals::WORLD_LENGTH;
    }
    std::unordered_map<Region_Key, Region> rmap;
    void build_section(long x, long y, Level& l) {
        auto find_itr = rmap.find({x, y});
        if(find_itr == rmap.end()) {
            long snap_x = align_to_region(x);
            long snap_y = align_to_region(y);
            auto new_seed_x = rmap.find({0, 0})->second.seed_x + snap_x;
            auto new_seed_y = rmap.find({0, 0})->second.seed_y + snap_y;
            find_itr =
                rmap.emplace(Region_Key{x, y},
                             Region(new_seed_x, new_seed_y,
                                    Rect(snap_x, snap_y, globals::WORLD_LENGTH,
                                         globals::WORLD_LENGTH)))
                    .first;
        }

        Section_Plan* search_sp = find_itr->second.section_plan(x, y, l.depth);
        if(search_sp && !search_sp->in_construction) {
            search_sp->in_construction = true;
            if(l.map.chunk_built(x, y)) return;
            search_sp->build_section(l);
        }
    }

    Regions() : rmap() {
        rmap.emplace(Region_Key{0, 0},
                     Region(Rect(0, 0, globals::WORLD_LENGTH,
                                 globals::WORLD_LENGTH)));
    }

    Regions(uint32_t seed_x, uint32_t seed_y) : rmap() {
        rmap.emplace(Region_Key{0, 0},
                     Region(seed_x, seed_y,
                            Rect(0, 0, globals::WORLD_LENGTH,
                                 globals::WORLD_LENGTH)));
    }
};

class MapWorld {
   public:
    std::vector<Level> levels;
    Regions regions;
    MapWindow map_window;
    ulong current_depth = 0;
    ItemInfoMap item_info_map = {};
    ulong instr_action_clock = 0;

    MapWorld(const Rect& border,
             bool is_walls_enabled);  // guaranteed first world
    MapWorld(const Rect& border, bool is_walls_enabled, uint32_t seed_x,
             uint32_t seed_y);
    MapWorld(const ant_proto::MapWorld& msg,
             ThreadPool<AsyncProgramJob>& thread_pool, bool is_walls_enabled);
    ant_proto::MapWorld get_proto() const;

    bool get_origin_region_seeds(uint32_t& seed_x, uint32_t& seed_y) const;

    Level& current_level();
    Level& operator[](ulong depth);
};
