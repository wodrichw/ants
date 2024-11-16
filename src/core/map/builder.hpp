#pragma once

#include <libtcod/bsp.hpp>

#include "map/section_data.hpp"

class BspListener : public ITCODBspCallback {
   private:
    MapSectionData &section_data;  // a section_data to dig
    int room_num = 0;                  // room number
    int lastx = 0, lasty = 0;              // center of the last room

   public:
    BspListener(MapSectionData &section_data);

    bool visitNode(TCODBsp *node, void *user_data);
};

struct RandomMapBuilder {
    Rect border;
    RandomMapBuilder(Rect const &border);
    void operator()(MapSectionData &section_data) const;
};

class FileMapBuilder {
    MapSectionData section_data;
    std::function<Rect(long, long, long, long)> rect_parser;

   public:
    FileMapBuilder(const std::string &filename);
    void operator()(MapSectionData &section_data) const;

   private:
    void load_file(const std::string &filename);
};

