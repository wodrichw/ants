#include "hardware/program_executor.hpp"

#include <string>

#include "app/globals.hpp"
#include "entity/entity_data.hpp"
#include "entity/map.hpp"
#include "spdlog/spdlog.h"

ProgramExecutor::ProgramExecutor(Unpacker&) {}

void ProgramExecutor::handleClockPulse() {
    // SPDLOG_TRACE("Handling clock pulse for program_executor");
    bool op_result = true;
    for (int i = 0; i < 500 && op_idx < _ops.size() && op_result; ++i) {
        op_result = _ops[op_idx]();
        ++op_idx;
        SPDLOG_TRACE("Incrementing op_idx to {}", op_idx);
    }

    // SPDLOG_TRACE("Clock pulse handled for program_executor");
}

Packer& operator<<(Packer& p, ProgramExecutor const&) {
    return p; 
}
