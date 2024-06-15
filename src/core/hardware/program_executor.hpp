#pragma once

#include <functional>
#include <string>
#include <unordered_map>
#include <vector>

using ulong = unsigned long;

class Packer;
class Unpacker;

struct ProgramExecutor {
   public:
    std::vector<std::function<ushort()>> _ops;
    ushort op_idx;
    ulong instr_trigger;
    ulong const& instr_clock;

    ProgramExecutor(ulong const& instr_clock);
    ProgramExecutor(Unpacker& p, ulong const& instr_clock); 
    void handleClockPulse();

    friend Packer& operator<<(Packer& p, ProgramExecutor const& obj);
};
