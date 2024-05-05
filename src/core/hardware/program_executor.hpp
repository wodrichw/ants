#pragma once

#include <functional>
#include <string>
#include <unordered_map>
#include <vector>

#include "hardware/controller.hpp"

class Packer;
class Unpacker;

struct ProgramExecutor: public ClockController {
   public:
    std::vector<std::function<bool()>> _ops;
    ushort op_idx;
    
    void handleClockPulse();

    friend Packer& operator<<(Packer& p, ProgramExecutor const& obj);
    friend Unpacker& operator>>(Unpacker& p, ProgramExecutor& obj);
};
