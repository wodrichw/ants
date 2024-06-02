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

    ProgramExecutor()=default;
    ProgramExecutor(Unpacker& p); 
    void handleClockPulse();
    ControllerType get_type() const { return PROGRAM_EXECUTOR; }

    friend Packer& operator<<(Packer& p, ProgramExecutor const& obj);
};
