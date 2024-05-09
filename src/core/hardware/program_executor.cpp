#include "hardware/program_executor.hpp"

#include "entity/entity_data.hpp"
#include "proto/hardware.pb.h"
#include "spdlog/spdlog.h"
#include "utils/serializer.hpp"

ProgramExecutor::ProgramExecutor(Unpacker& p) {
    ant_proto::ProgramExecutor msg;
    p >> msg;

    op_idx = msg.op_idx();
}

void ProgramExecutor::handleClockPulse() {
    // SPDLOG_TRACE("Handling clock pulse for program_executor");
    bool op_result = true;
    for(int i = 0; i < 500 && op_idx < _ops.size() && op_result; ++i) {
        op_result = _ops[op_idx]();
        ++op_idx;
        SPDLOG_TRACE("Incrementing op_idx to {}", op_idx);
    }

    // SPDLOG_TRACE("Clock pulse handled for program_executor");
}

Packer& operator<<(Packer& p, ProgramExecutor const& obj) {
    ant_proto::ProgramExecutor msg;
    msg.set_op_idx(obj.op_idx);
    return p << msg;
}
