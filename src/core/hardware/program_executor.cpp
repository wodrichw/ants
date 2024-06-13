#include "hardware/program_executor.hpp"

#include "entity/entity_data.hpp"
#include "proto/hardware.pb.h"
#include "spdlog/spdlog.h"
#include "utils/serializer.hpp"

ProgramExecutor::ProgramExecutor(Unpacker& p): skip_count(0) {
    ant_proto::ProgramExecutor msg;
    p >> msg;

    op_idx = msg.op_idx();
    SPDLOG_TRACE("Completed unpacking program executor - op_idx: {}", op_idx);
}

void ProgramExecutor::handleClockPulse() {
    // SPDLOG_TRACE("Handling clock pulse for program_executor - skip count: {}", skip_count);
    update_skip_count();
    for(int i = 0; i < 500 && op_idx < _ops.size() && skip_count == 0; ++i) {
        skip_count = _ops[op_idx]();
        ++op_idx;
        SPDLOG_TRACE("Incrementing op_idx to {}", op_idx);
    }

    // SPDLOG_TRACE("Clock pulse handled for program_executor");
}

void ProgramExecutor::update_skip_count() {
    if(skip_count > 0) --skip_count;
}

Packer& operator<<(Packer& p, ProgramExecutor const& obj) {
    SPDLOG_TRACE("Packing program executor - op_idx: {}", obj.op_idx);
    ant_proto::ProgramExecutor msg;
    msg.set_op_idx(obj.op_idx);
    return p << msg;
}
