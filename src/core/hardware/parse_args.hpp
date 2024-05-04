#pragma once

#include <sstream>

#include "hardware/parser.hpp"

struct DualRegisters;
struct MapEntity;
class Map;
class Operations;

struct ParserArgs {
    DualRegisters& registers;
    MapEntity& entity;
    Map& map;
    Operations& operations;
    ParserArgs(DualRegisters& registers, MapEntity& entity, Map& map,
               Operations& operations): registers(registers), entity(entity),
                map(map), operations(operations) {}
};

struct ParseLineArgs {
    std::istringstream &code_stream;
    ParserStatus &status;
    ParserArgs& args;
};