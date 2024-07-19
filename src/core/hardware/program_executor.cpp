#include "hardware/program_executor.hpp"

#include "entity/entity_data.hpp"
#include "proto/hardware.pb.h"
#include "spdlog/spdlog.h"
#include "utils/serializer.hpp"

ProgramExecutor::ProgramExecutor(
    ulong const& instr_clock,
    ulong max_instruction_per_tick,
    ThreadPool<AsyncProgramJob>& job_pool
):
    op_idx(0),
    instr_trigger(0),
    has_executed_async(false),
    has_executed_sync(false),
    instr_clock(instr_clock),
    max_instruction_per_tick(max_instruction_per_tick),
    job_pool(job_pool)
{}

ProgramExecutor::ProgramExecutor(
    Unpacker& p,
    ulong const& instr_clock,
    ulong max_instruction_per_tick,
    ThreadPool<AsyncProgramJob>& job_pool
):
    instr_clock(instr_clock),
    max_instruction_per_tick(max_instruction_per_tick),
    job_pool(job_pool)
        
{
    ant_proto::ProgramExecutor msg;
    p >> msg;

    op_idx = msg.op_idx();
    instr_trigger = msg.instr_trigger();
    has_executed_sync = msg.has_executed();
    SPDLOG_TRACE("Completed unpacking program executor - op_idx: {}", op_idx);
}

void ProgramExecutor::reset() { has_executed_sync = false; }




void ProgramExecutor::execute_async() {
    // SPDLOG_INFO("Handling clock pulse for program_executor - clock: {} trigger: {}", instr_clock, instr_trigger);
    has_executed_async = false;
    if( op_idx >= _ops.size() ) return;
    if ((instr_clock % (instr_trigger + 1)) != 0) return;
    instr_trigger = 0; // if not 0, then a syncronous move is occurring
    has_executed_async = true;
    AsyncProgramJob job(*this);
    job_pool.submit_job(job);

    SPDLOG_TRACE("submitted async op {}", op_idx);
}


void ProgramExecutor::execute() {
    instr_trigger = _ops[op_idx].num_ticks;
    _ops[op_idx]();
    op_idx++;
}

void ProgramExecutor::execute_sync() {
    if( has_executed_sync )  return;
    if( op_idx >= _ops.size() ) return;
    if(! has_executed_async ) return;
    has_executed_sync = true;
    SPDLOG_TRACE("executing sync op {}", op_idx);
    execute();
}

bool ProgramExecutor::is_sync() {
    return _ops[op_idx].num_ticks != 0;
}

void AsyncProgramJob::run() {
	for(ulong i = 1; i < pe.max_instruction_per_tick && pe.op_idx < pe._ops.size(); ++i) {
		if( pe.is_sync() ) { // break if a syncronous instruction
			break;
		}
        SPDLOG_TRACE("executing async op {}", pe.op_idx);
		pe.execute();
		SPDLOG_TRACE("Incrementing op_idx to {}", pe.op_idx);
	}
}

Packer& operator<<(Packer& p, ProgramExecutor const& obj) {
    SPDLOG_TRACE("Packing program executor - op_idx: {}", obj.op_idx);
    ant_proto::ProgramExecutor msg;
    msg.set_op_idx(obj.op_idx);
    msg.set_instr_trigger(obj.instr_trigger);
    msg.set_has_executed(obj.has_executed_sync);
    return p << msg;
}
