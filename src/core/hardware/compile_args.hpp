#pragma once

#include <sstream>

#include "hardware/parser.hpp"

using uchar = unsigned char;
using ulong = unsigned long;

struct DualRegisters;
struct MapEntity;
class Map;
struct ProgramExecutor;
struct Status;

struct AntInteractor {
    DualRegisters& registers;
    MapEntity& entity;
    Map& map;
    std::vector<std::function<ushort()>>& ops;
    ushort& op_idx;
    ushort move_speed;
    AntInteractor(DualRegisters& registers, MapEntity& entity, Map& map,
               std::vector<std::function<ushort()>>& ops, ushort& op_idx, ushort move_speed): registers(registers), entity(entity),
                map(map), ops(ops), op_idx(op_idx), move_speed(move_speed) {}
};

struct CompileArgs {
    std::vector<uchar>::const_iterator code_it;
    AntInteractor& interactor;
    Status& status;
};
