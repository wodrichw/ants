#include <libtcod.hpp>
#include <libtcod/bsp.hpp>
#include <libtcod/color.hpp>
#include <libtcod/console_drawing.h>
#include <vector>

#include "map.hpp"
#include "ant.hpp"
#include "building.hpp"
#include "colors.hpp"

class BspListener : public ITCODBspCallback {
private:
  Map &map;         // a map to dig
  int roomNum;      // room number
  int lastx, lasty; // center of the last room

public:
  BspListener(Map &map) : map(map), roomNum(0) {}

  bool visitNode(TCODBsp *node, void *user_data) {
    (void)user_data; // don't need user_data
    if (node->isLeaf()) {
      int x, y, w, h;
      // dig a room
      TCODRandom *rng = TCODRandom::getInstance();
      w = rng->getInt(ROOM_MIN_SIZE, node->w - 2);
      h = rng->getInt(ROOM_MIN_SIZE, node->h - 2);
      x = rng->getInt(node->x + 1, node->x + node->w - w - 1);
      y = rng->getInt(node->y + 1, node->y + node->h - h - 1);
      map.createRoom(roomNum == 0, x, y, x + w - 1, y + h - 1);
      if (roomNum != 0) {
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

Map::Map(int width, int height, std::vector<ant::Ant *> &ants,
         std::vector<Building *> &buildings)
    : width(width), height(height), ants(ants), buildings(buildings),
      tiles(new Tile[width * height]), map(new TCODMap(width, height)) {
  TCODBsp bsp(0, 0, width, height); // bsp is binary space partition tree
  // this creates the room partitions in our map
  int nb = 8; // max level of recursion -- can make 2^nb rooms.
  bsp.splitRecursive(NULL, nb, ROOM_MAX_SIZE, ROOM_MAX_SIZE, 1.5f, 1.5f);
  BspListener listener(*this);
  bsp.traverseInvertedLevelOrder(&listener, NULL);
}

Map::~Map() {
  delete[] tiles;
  delete map;
}

Tile &Map::getTile(int x, int y) const { return tiles[x + y * width]; }

void Map::clearCh(int x, int y) {
  auto &tile = root_console.at(x, y);
  tile.ch = ' ';
}

void Map::setWall(int x, int y) { map->setProperties(x, y, false, false); }

bool Map::isWall(int x, int y) const { return !map->isWalkable(x, y); }

bool Map::canWalk(int x, int y) const {
  if (isWall(x, y)) {
    return false;
  }
  for (auto ant : ants) {
    if (ant->x == x && ant->y == y) {
      // there is an actor there. cannot walk
      return false;
    }
  }
  return true;
}

bool Map::isInFov(int x, int y) const {
  Tile &tile = getTile(x, y);
  if (tile.inFov) {
    tile.explored = true;
    return true;
  }
  return false;
}

bool Map::isExplored(int x, int y) const { return getTile(x, y).explored; }

void Map::render() {
  TCOD_ColorRGBA darkWall = color::light_black;
  TCOD_ColorRGBA darkGround = color::dark_grey;
  TCOD_ColorRGBA lightWall = color::indian_red;
  TCOD_ColorRGBA lightGround = color::grey;

  for (int x = 0; x < width; x++) {
    for (int y = 0; y < height; y++) {
      auto &tile = root_console.at(x, y);
      tile.ch = ' ';
      if (isInFov(x, y)) {
        tile.bg = isWall(x, y) ? lightWall : lightGround;
      } else {
        if (isExplored(x, y)) {
          tile.bg = isWall(x, y) ? darkWall : darkGround;
        } else {
          tile.bg = darkWall;
        }
      }
    }
  }
}

void Map::renderAnt(ant::Ant &a) {
  if (isInFov(a.x, a.y)) {
    auto &tile = root_console.at(a.x, a.y);
    tile.ch = a.ch;
    tile.fg = a.col;
  }
}

void Map::renderBuilding(Building &b) {
  for (int xi = b.x; xi < b.x + b.w; ++xi) {
    for (int yi = b.y; yi < b.y + b.h; ++yi) {
      auto &tile = root_console.at(xi, yi);
      tile.bg = b.color;
    }
  }
}

void Map::resetFov() {
  for (int x = 0; x < width; x++) {
    for (int y = 0; y < height; y++) {
      getTile(x, y).inFov = false;
    }
  }
  for (auto ant : ants) {
    ant->resetFov();
  }
}

void Map::updateTileFov() {
  for (int x = 0; x < width; x++) {
    for (int y = 0; y < height; y++) {
      if (map->isInFov(x, y)) {
        getTile(x, y).inFov = true;
      }
    }
  }
}

void Map::updateFov() {
  resetFov();
  for (auto ant : ants) {
    map->computeFov(ant->x, ant->y, ant->fovRadius);
    updateTileFov();
  }
}

/*
 * dig makes a rectangle of map specified by two x,y points as walkable
 */
void Map::dig(int x1, int y1, int x2, int y2) {
  if (x2 < x1) {
    int tmp = x2;
    x2 = x1;
    x1 = tmp;
  }
  if (y2 < y1) {
    int tmp = y2;
    y2 = y1;
    y1 = tmp;
  }
  for (int tilex = x1; tilex <= x2; tilex++) {
    for (int tiley = y1; tiley <= y2; tiley++) {
      map->setProperties(tilex, tiley, true, true);
    }
  }
}

// TODO: make a table which keeps track of rooms
// As rooms are constructed, add room dimensions to table
// Then, once all rooms are constructed, we can add in objects, monsters,
// buildings, and ants This should stop ants and buildnigs references from being
// passed intp Map class
void Map::createRoom(bool first, int x1, int y1, int x2, int y2) {
  dig(x1, y1, x2, y2);

  if (first) {
    int center_x = (x1+x2)/2, center_y = (y1+y2)/2;
    // put the player in the first room
    ants[0]->x = center_x;
    ants[0]->y = center_y;
    buildings.push_back(new Nursery(center_x, center_y, 0, *this));
  } else {
    // TCODRandom *rng=TCODRandom::getInstance();
    // int nbAnts=rng->getInt(0,MAX_ROOM_MONSTERS);
    // while (nbAnts > 0) {
    //     int x=rng->getInt(x1,x2);
    //     int y=rng->getInt(y1,y2);
    //     if ( canWalk(x,y) ) {
    //         ants.push_back(new ant::Player(x, y, 2, 'w', tcod::ColorRGB{1, 2,
    //         3}));
    //     }
    //     nbAnts--;
    // }
  }
}


