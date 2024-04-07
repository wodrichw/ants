#include "map.hpp"

#include <libtcod/console_drawing.h>
#include <libtcod.hpp>
#include <libtcod/bsp.hpp>
#include <libtcod/color.hpp>

#include <vector>
#include <fstream>
#include <iostream>
#include <sstream>

#include "ant.hpp"
#include "building.hpp"
#include "arg_parse.hpp"

using ulong = unsigned long;

class BspListener : public ITCODBspCallback {
   private:
    Map &map;          // a map to dig
    int roomNum;       // room number
    int lastx, lasty;  // center of the last room

   public:
    BspListener(Map &map) : map(map), roomNum(0) {}

    bool visitNode(TCODBsp *node, void *user_data) {
        (void)user_data;  // don't need user_data
        if(node->isLeaf()) {
            long x, y, w, h;
            // dig a room
            TCODRandom *rng = TCODRandom::getInstance();
            w = rng->getInt(ROOM_MIN_SIZE, node->w - 2);
            h = rng->getInt(ROOM_MIN_SIZE, node->h - 2);
            x = rng->getInt(node->x + 1, node->x + node->w - w - 1);
            y = rng->getInt(node->y + 1, node->y + node->h - h - 1);
            map.createRoom(roomNum == 0, x, y, x + w - 1, y + h - 1);
            if(roomNum != 0) {
                // dig a corridor from last room
                map.dig(lastx, lasty, x + w / 2, lasty);
                map.dig(x + w / 2, lasty, x + w / 2, y + h / 2);
            }
            lastx = x + w / 2;
            lasty = y + h / 2;
            roomNum++;
        }
        return true;
    }
};

struct RandomMapBuilder {
    void operator()(Map &map) {
        TCODBsp bsp(0, 0, map.width, map.height);  // bsp is binary space partition tree
        // this creates the room partitions in our map
        int nb = 8;  // max level of recursion -- can make 2^nb rooms.
        bsp.splitRecursive(NULL, nb, ROOM_MAX_SIZE, ROOM_MAX_SIZE, 1.5f, 1.5f);
        BspListener listener(map);
        bsp.traverseInvertedLevelOrder(&listener, NULL);
    }
};

struct RoomRect {
    long x1, y1, w, h, x2, y2;
    long center_x, center_y;
    RoomRect(long x1, long y1, long w, long h) : x1(x1), y1(y1), w(w), h(h),
        x2(x1 + w - 1), y2(y1 + h - 1),
        center_x(x1 + w / 2), center_y(y1 + h / 2) {
            // std::cout << "New Rect: " << x1 << ", " << y1 << ", " << w << ", " << h << std::endl;
        }
    
    static RoomRect from_top_left(long x1, long y1, long w, long h) {
        return RoomRect(x1, y1, w, h);
    }
    
    static RoomRect from_center(long center_x, long center_y, long w, long h) {
        return RoomRect(center_x - w / 2, center_y - h / 2, w, h);
    }
    
    static RoomRect from_corners(long x1, long y1, long x2, long y2) {
        return RoomRect(x1, y2, x2 - x1 + 1, y2 - y1 + 1);
    }
};

class FileMapBuilder {
    std::vector<RoomRect> rooms;
    std::vector<std::tuple<long, long>> corridors;
    std::function<RoomRect(long, long, long, long)> rect_parser;
public:
    FileMapBuilder(const std::string &filename) { load_file(filename); }

    void load_file(const std::string &filename) {
        // load the map from a file
        std::ifstream file(filename);
        if (!file) {
            std::cerr << "Unable to open file: " << filename << std::endl;
            return;
        }
        // std::cout << "Loading map from file: " << filename << std::endl;

        // get the read mode
        std::string read_mode; // CENTER or TOP_LEFT or CORNERS
        file >> read_mode;
        rect_parser = read_mode == "CENTER" ? RoomRect::from_center : (read_mode == "CORNERS" ? RoomRect::from_corners : RoomRect::from_top_left);
        // std::cout << "Read mode: " << read_mode << std::endl;

        // get the room and corridor count
        long room_count, corridor_count;
        file >> room_count >> corridor_count;

        // read the rooms
        for (long i = 0; i <  room_count; ++i) {
            long x, y, w, h;
            file >> x >> y >> w >> h;
            rooms.push_back(rect_parser(x, y, w, h));
        }

        // read the corridors
        for (long i = 0; i <  corridor_count; ++i) {
            long room_idx1, room_idx2;
            file >> room_idx1 >> room_idx2;
            corridors.push_back(std::make_pair(room_idx1, room_idx2));
        }
    }
    void operator()(Map &map) {
        bool first = true;
        for (auto &room : rooms) {
            map.createRoom(first, room.x1, room.y1, room.x2, room.y2);
            first = false;
        }

        for (auto &corridor : corridors) {
            long room_idx1, room_idx2;
            std::tie(room_idx1, room_idx2) = corridor;
            RoomRect &r1 = rooms[room_idx1];
            RoomRect &r2 = rooms[room_idx2];

            bool vertical_gap = r1.y2 < r2.y1 || r2.y2 < r1.y1;
            bool horizontal_gap = r1.x2 < r2.x1 || r2.x2 < r1.x1;

            if (vertical_gap && horizontal_gap) {
                // no overlap - L-shaped corridor
                // std::cout << "Digging corridor between room " << room_idx1 << " and " << room_idx2 << " (L-shaped) - " << r1.x1 << ", " << r1.y1 << " -> " << r2.x1 << ", " << r2.y1 << "\n";
                map.dig(r1.center_x, r1.center_y, r2.center_x, r1.center_y);
                map.dig(r2.center_x, r1.center_y, r2.center_x, r2.center_y);
            } else if (vertical_gap) {
                // no overlap - vertical gap
                long min_x = std::max(r1.x1, r2.x1), max_x = std::min(r1.x2, r2.x2);
                long center_x = (min_x + max_x) / 2;
                // std::cout << "Digging corridor between room " << room_idx1 << " and " << room_idx2 << " (vertical) - " << center_x << ", " << r1.center_y << " -> " << center_x << ", " << r2.center_y << "\n";
                map.dig(center_x, r1.center_y, center_x, r2.center_y);
            } else if (horizontal_gap) {
                // no overlap - horizontal gap
                // std::cout << "Digging corridor between room " << room_idx1 << " and " << room_idx2 << " (horizontal)\n";
                long min_y = std::max(r1.y1, r2.y1), max_y = std::min(r1.y2, r2.y2);
                long center_y = (min_y + max_y) / 2;
                map.dig(r1.center_x, center_y, r2.center_x, center_y);
            } else {
                // overlap
                std::cerr << "Overlapping rooms not supported between room " << room_idx1 << " and " << room_idx2 << std::endl;
            }
        }
    }
};

Map::Map(int width, int height, std::vector<Ant *> &ants,
         std::vector<Building *> &buildings, ProjectArguments &config) :
      width(width),
      height(height),
      ants(ants),
      buildings(buildings),
      tiles(new Tile[width * height]),
      map(new TCODMap(width, height)) {
        if (config.default_map_file_path.empty()) build_map = RandomMapBuilder();
        else build_map = FileMapBuilder(config.default_map_file_path);
      }

Map::~Map() {
    delete[] tiles;
    delete map;
}

void Map::build() { build_map(*this); }

Tile &Map::getTile(long x, long y) const { return tiles[x + y * width]; }

void Map::setWall(long x, long y) { map->setProperties(x, y, false, false); }

bool Map::isWall(long x, long y) const { return !map->isWalkable(x, y); }

bool Map::canWalk(long x, long y) const {
    if(x < 0 || x >= width || y < 0 || y >= height) return false;
    if(isWall(x, y)) {
        return false;
    }
    for(auto ant : ants) {
        if(ant->x == x && ant->y == y) {
            // there is an actor there. cannot walk
            return false;
        }
    }
    return true;
}

bool Map::isInFov(long x, long y) const {
    Tile &tile = getTile(x, y);
    if(tile.inFov) {
        tile.explored = true;
        return true;
    }
    return false;
}

bool Map::isExplored(long x, long y) const { return getTile(x, y).explored; }

void Map::resetFov() {
    for(long x = 0; x < width; x++) {
        for(long y = 0; y < height; y++) {
            getTile(x, y).inFov = false;
        }
    }
    for(auto ant : ants) {
        ant->resetFov();
    }
}

void Map::updateTileFov() {
    for(long x = 0; x < width; x++) {
        for(long y = 0; y < height; y++) {
            if(map->isInFov(x, y)) {
                getTile(x, y).inFov = true;
            }
        }
    }
}

void Map::updateFov() {
    resetFov();
    for(auto ant : ants) {
        map->computeFov(ant->x, ant->y, ant->fovRadius);
        updateTileFov();
    }
}

/*
 * dig makes a rectangle of map specified by two x,y points as walkable
 */
void Map::dig(long x1, long y1, long x2, long y2) {
    if(x2 < x1) {
        int tmp = x2;
        x2 = x1;
        x1 = tmp;
    }
    if(y2 < y1) {
        int tmp = y2;
        y2 = y1;
        y1 = tmp;
    }
    for(int tilex = x1; tilex <= x2; tilex++) {
        for(int tiley = y1; tiley <= y2; tiley++) {
            map->setProperties(tilex, tiley, true, true);
        }
    }
}

// TODO: make a table which keeps track of rooms
// As rooms are constructed, add room dimensions to table
// Then, once all rooms are constructed, we can add in objects, monsters,
// buildings, and ants This should stop ants and buildnigs references from being
// passed intp Map class
void Map::createRoom(bool first, long x1, long y1, long x2, long y2) {
    dig(x1, y1, x2, y2);

    if(first) {
        int center_x = (x1 + x2) / 2, center_y = (y1 + y2) / 2;
        // put the player in the first room
        ants[0]->x = center_x;
        ants[0]->y = center_y;
        buildings.push_back(new Nursery(center_x, center_y, 0, *this));
    }
}
