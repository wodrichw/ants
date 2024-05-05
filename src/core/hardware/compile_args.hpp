#pragma once

#include <sstream>

#include "hardware/parser.hpp"

using uchar = unsigned char;

struct DualRegisters;
struct MapEntity;
class Map;
struct ProgramExecutor;
struct Status;

struct AntInteractor {
    DualRegisters& registers;
    MapEntity& entity;
    Map& map;
    std::vector<std::function<bool()>>& ops;
    ushort& op_idx;
    AntInteractor(DualRegisters& registers, MapEntity& entity, Map& map,
               std::vector<std::function<bool()>>& ops, ushort& op_idx): registers(registers), entity(entity),
                map(map), ops(ops), op_idx(op_idx) {}
};

struct CompileArgs {
    std::vector<uchar>::const_iterator code_it;
    AntInteractor& interactor;
    Status& status;
};
