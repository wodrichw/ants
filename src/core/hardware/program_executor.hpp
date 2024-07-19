#pragma once

#include <functional>
#include <vector>

#include "utils/thread_pool.hpp"
#include "hardware/op_def.hpp"
#include "app/globals.hpp"

using ulong = unsigned long;

class Packer;
class Unpacker;
struct ProgramExecutor;

struct Op {
    unsigned short num_ticks;
    std::function< void() > fn;
    void operator()() { fn() ; };
};

struct AsyncProgramJob {
    ProgramExecutor& pe;
    AsyncProgramJob( ProgramExecutor& pe ): pe(pe) {}
    void run();
};

struct ProgramExecutor {
   public:
    std::vector<Op> _ops;
    unsigned short op_idx = 0;
    ulong instr_trigger = 0;
    bool has_executed_async = false;
    bool has_executed_sync = false;
    ulong const& instr_clock;
    ulong max_instruction_per_tick = 0;
    ThreadPool<AsyncProgramJob>& job_pool;

    ProgramExecutor(ulong const& instr_clock, ulong max_instruction_per_tick, ThreadPool<AsyncProgramJob>&);
    ProgramExecutor(Unpacker& p, ulong const& instr_clock, ulong max_instruction_per_tick, ThreadPool<AsyncProgramJob>&);
    void reset();
    void execute_async();
    void execute();
    void execute_sync();
    bool is_sync();

    friend Packer& operator<<(Packer& p, ProgramExecutor const& obj);
};


