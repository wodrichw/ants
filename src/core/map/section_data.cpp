#include "map/section_data.hpp"

MapSectionData::MapSectionData(MapSectionData const &other)
    : border(other.border) {
    for(auto &room : other.rooms) {
        rooms.push_back(room);
    }

    for(auto &corridor : other.corridors) {
        corridors.push_back(corridor);
    }
}

MapSectionData &MapSectionData::operator=(MapSectionData const &other) {
    border = other.border;
    rooms.clear();
    corridors.clear();

    for(auto &room : other.rooms) {
        rooms.push_back(room);
    }

    for(auto &corridor : other.corridors) {
        corridors.push_back(corridor);
    }

    return *this;
}
