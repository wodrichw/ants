#include "hardware/program_executor.hpp"

#include "entity/entity_data.hpp"
#include "proto/hardware.pb.h"
#include "spdlog/spdlog.h"
#include "utils/serializer.hpp"

ProgramExecutor::ProgramExecutor(ulong const& instr_clock) : instr_trigger(0), instr_clock(instr_clock) {}

ProgramExecutor::ProgramExecutor(Unpacker& p, ulong const& instr_clock): instr_clock(instr_clock) {
    ant_proto::ProgramExecutor msg;
    p >> msg;

    op_idx = msg.op_idx();
    instr_trigger = msg.instr_trigger();
    SPDLOG_TRACE("Completed unpacking program executor - op_idx: {}", op_idx);
}

void ProgramExecutor::handleClockPulse() {
    // SPDLOG_INFO("Handling clock pulse for program_executor - clock: {} trigger: {} EXEC: {}", instr_clock, instr_trigger, (instr_clock % instr_trigger) == 0 ? "YES" : "NO");
    if ((instr_clock % (instr_trigger + 1)) != 0) return;

    instr_trigger = 0;
    for(int i = 0; i < 500 && op_idx < _ops.size() && instr_trigger == 0; ++i) {
        instr_trigger = _ops[op_idx]();
        ++op_idx;
        SPDLOG_TRACE("Incrementing op_idx to {}", op_idx);
    }

    // SPDLOG_TRACE("Clock pulse handled for program_executor");
}

Packer& operator<<(Packer& p, ProgramExecutor const& obj) {
    SPDLOG_TRACE("Packing program executor - op_idx: {}", obj.op_idx);
    ant_proto::ProgramExecutor msg;
    msg.set_op_idx(obj.op_idx);
    msg.set_instr_trigger(obj.instr_trigger);
    return p << msg;
}
