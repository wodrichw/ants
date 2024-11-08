#include <fstream>

#include "entity/map_builder.hpp"
#include "spdlog/spdlog.h"

BspListener::BspListener(MapSectionData &section_data) :
    section_data(section_data), room_num(0)
{
    SPDLOG_TRACE("BspListener created");
}

bool BspListener::visitNode(TCODBsp *node, void *user_data) {
    (void)user_data;  // don't need user_data
    if(!node->isLeaf()) return true;

    // dig a room
    TCODRandom *rng = TCODRandom::getInstance();
    long w = rng->getInt(ROOM_MIN_SIZE, node->w - 2);
    long h = rng->getInt(ROOM_MIN_SIZE, node->h - 2);
    long x = rng->getInt(node->x + 1, node->x + node->w - w - 1);
    long y = rng->getInt(node->y + 1, node->y + node->h - h - 1);

    SPDLOG_TRACE("Creating room {} at ({}, {}) to ({}, {})", room_num, x, y,
                 x + w - 1, y + h - 1);
    section_data.rooms.push_back(Rect::from_top_left(x, y, w, h));

    if(room_num != 0) {
        SPDLOG_DEBUG("Digging corridor from ({}, {}) to ({}, {})", lastx,
                     lasty, x + w / 2, lasty);
        // dig a corridor from last room
        section_data.corridors.push_back(
            Rect::from_corners(lastx, lasty, x + w / 2, lasty));
        section_data.corridors.push_back(
            Rect::from_corners(x + w / 2, lasty, x + w / 2, y + h / 2));
    }
    lastx = x + w / 2;
    lasty = y + h / 2;
    room_num++;

    return true;
}

RandomMapBuilder::RandomMapBuilder(Rect const &border) : border(border) {
    SPDLOG_INFO("Creating RandomMapBuilder");
}

void RandomMapBuilder::operator()(MapSectionData &section_data) const {
    SPDLOG_INFO(
        "Building random section_data at {}x{} - max room size: {} max "
        "ratio: {}",
        section_data.border.w, section_data.border.h, ROOM_MAX_SIZE, 1.5f);
    section_data.border = border;

    TCODBsp bsp(
        0, 0, section_data.border.w,
        section_data.border.h);  // bsp is binary space partition tree
    // this creates the room partitions in our section_data
    int nb = 8;  // max level of recursion -- can make 2^nb rooms.

    // TODO: save seeds by changing from NULL being passed into splitRecursive to 
    // saved seed value
    bsp.splitRecursive(NULL, nb, ROOM_MAX_SIZE, ROOM_MAX_SIZE, 1.5f, 1.5f);
    BspListener listener(section_data);
    bsp.traverseInvertedLevelOrder(&listener, NULL);
}

FileMapBuilder::FileMapBuilder(const std::string &filename) {
    SPDLOG_INFO("Creating FileMapBuilder");
    load_file(filename);
}

void FileMapBuilder::operator()(MapSectionData &section_data) const {
    SPDLOG_DEBUG(
        "Copying section_data from FileMapBuilder to section_data");
    section_data = this->section_data;
}

void FileMapBuilder::load_file(const std::string &filename) {
    SPDLOG_INFO("Loading section_data from file: {}", filename);

    // load the section_data from a file
    std::ifstream file(filename);
    if(!file) {
        SPDLOG_ERROR("Unable to open file: {}", filename);
        return;
    }
    SPDLOG_DEBUG("Successfully opened file: {}", filename);

    // get the read mode
    std::string read_mode;  // CENTER or TOP_LEFT or CORNERS
    file >> read_mode;
    rect_parser = read_mode == "CENTER"
                      ? Rect::from_center
                      : (read_mode == "CORNERS" ? Rect::from_corners
                                                : Rect::from_top_left);
    SPDLOG_DEBUG("Rect read mode for the file: {}", read_mode);

    // get the border
    long a, b, c, d;
    file >> a >> b >> c >> d;
    section_data.border = rect_parser(a, b, c, d);

    // get the room and corridor count
    long room_count, corridor_count;
    file >> room_count >> corridor_count;
    SPDLOG_DEBUG("Room count: {}, Corridor count: {}", room_count,
                 corridor_count);

    auto &rooms = section_data.rooms;
    auto &corridors = section_data.corridors;

    // read the rooms
    SPDLOG_DEBUG("Reading rooms");
    for(long i = 0; i < room_count; ++i) {
        long x, y, w, h;
        file >> x >> y >> w >> h;
        rooms.push_back(rect_parser(x, y, w, h));
    }

    // read the corridors
    SPDLOG_DEBUG("Reading corridors");
    for(long i = 0; i < corridor_count; ++i) {
        long room_idx1, room_idx2;
        file >> room_idx1 >> room_idx2;

        Rect &r1 = rooms[room_idx1];
        Rect &r2 = rooms[room_idx2];

        bool vertical_gap = r1.y2 < r2.y1 || r2.y2 < r1.y1;
        bool horizontal_gap = r1.x2 < r2.x1 || r2.x2 < r1.x1;
        SPDLOG_TRACE("Vertical gap: {}, Horizontal gap: {}", vertical_gap,
                     horizontal_gap);

        if(vertical_gap && horizontal_gap) {
            // no overlap - L-shaped corridor
            SPDLOG_TRACE(
                "Digging corridor between room {} and {} (L-shaped) - {}, "
                "{} -> {}, {}",
                room_idx1, room_idx2, r1.x1, r1.y1, r2.x1, r2.y1);
            corridors.push_back(Rect::from_corners(
                r1.center_x, r1.center_y, r2.center_x, r1.center_y));
            corridors.push_back(Rect::from_corners(
                r2.center_x, r1.center_y, r2.center_x, r2.center_y));
        } else if(vertical_gap) {
            // no overlap - vertical gap
            long min_x = std::max(r1.x1, r2.x1),
                 max_x = std::min(r1.x2, r2.x2);
            long center_x = (min_x + max_x) / 2;
            SPDLOG_TRACE(
                "Digging corridor between room {} and {} (vertical) - {}, "
                "{} -> {}, {}",
                room_idx1, room_idx2, center_x, r1.center_y, center_x,
                r2.center_y);
            corridors.push_back(Rect::from_corners(center_x, r1.center_y,
                                                   center_x, r2.center_y));
        } else if(horizontal_gap) {
            // no overlap - horizontal gap
            long min_y = std::max(r1.y1, r2.y1),
                 max_y = std::min(r1.y2, r2.y2);
            long center_y = (min_y + max_y) / 2;
            SPDLOG_TRACE(
                "Digging corridor between room {} and {} (horizontal) - "
                "{}, {} -> {}, {}",
                room_idx1, room_idx2, r1.center_x, center_y, r2.center_x,
                center_y);
            corridors.push_back(Rect::from_corners(r1.center_x, center_y,
                                                   r2.center_x, center_y));
        } else {
            // overlap
            SPDLOG_ERROR(
                "Overlapping rooms not supported between room {} and {}",
                room_idx1, room_idx2);
        }
    }
}

