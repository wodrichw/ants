#include "hardware/program_executor.hpp"

#include "entity/entity_data.hpp"
#include "proto/hardware.pb.h"
#include "spdlog/spdlog.h"
#include "utils/serializer.hpp"

ProgramExecutor::ProgramExecutor(ulong const& instr_clock,
                                 ulong max_instruction_per_tick,
                                 ushort& instr_ptr_register,
                                 ThreadPool<AsyncProgramJob>& job_pool)
    : instr_ptr_register(instr_ptr_register),
      instr_trigger(0),
      has_executed_async(false),
      has_executed_sync(false),
      instr_clock(instr_clock),
      max_instruction_per_tick(max_instruction_per_tick),
      job_pool(job_pool) {}

ProgramExecutor::ProgramExecutor(const ant_proto::ProgramExecutor& msg,
                                 ulong const& instr_clock,
                                 ulong max_instruction_per_tick,
                                 ushort& instr_ptr_register,
                                 ThreadPool<AsyncProgramJob>& job_pool)
    : instr_ptr_register(instr_ptr_register),
      instr_trigger(msg.instr_trigger()),
      has_executed_sync(msg.has_executed()),
      instr_clock(instr_clock),
      max_instruction_per_tick(max_instruction_per_tick),
      job_pool(job_pool)

{}

void ProgramExecutor::reset() { has_executed_sync = false; }

void ProgramExecutor::execute_async() {
    // SPDLOG_INFO("Handling clock pulse for program_executor - clock: {}
    // trigger: {}", instr_clock, instr_trigger);
    has_executed_async = false;
    if(instr_ptr_register >= _ops.size()) return;
    if((instr_clock % (instr_trigger + 1)) != 0) return;
    instr_trigger = 0;  // if not 0, then a syncronous move is occurring
    has_executed_async = true;
    AsyncProgramJob job(*this);
    job_pool.submit_job(job);

    SPDLOG_TRACE("Submitted async job - instruction address: {}",
                 instr_ptr_register);
}

void ProgramExecutor::execute() {
    instr_trigger = _ops[instr_ptr_register].num_ticks;
    _ops[instr_ptr_register]();
    ++instr_ptr_register;
}

void ProgramExecutor::execute_sync() {
    if(has_executed_sync) return;
    if(instr_ptr_register >= _ops.size()) return;
    if(!has_executed_async) return;
    has_executed_sync = true;
    SPDLOG_TRACE("Executing sync operation at instruction address: {}",
                 instr_ptr_register);
    execute();
}

bool ProgramExecutor::is_sync() {
    return _ops[instr_ptr_register].num_ticks != 0;
}

void AsyncProgramJob::run() {
    for(ulong i = 1; i < pe.max_instruction_per_tick &&
                     pe.instr_ptr_register < pe._ops.size();
        ++i) {
        if(pe.is_sync()) {  // break if a syncronous instruction
            break;
        }
        SPDLOG_TRACE("Executing async operation at instruction address: {}",
                     pe.instr_ptr_register);
        pe.execute();
        SPDLOG_TRACE("Incrementing instruction pointer register to {}",
                     pe.instr_ptr_register);
    }
}

ant_proto::ProgramExecutor ProgramExecutor::get_proto() {
    ant_proto::ProgramExecutor msg;
    msg.set_instr_trigger(instr_trigger);
    msg.set_has_executed(has_executed_sync);
    return msg;
}
