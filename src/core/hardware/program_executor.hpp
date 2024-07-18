#pragma once

#include "utils/thread_pool.hpp"
#include <vector>

using ulong = unsigned long;

class Packer;
class Unpacker;
struct ProgramExecutor;

struct Op {
    unsigned short num_ticks;
    void operator()();
};

struct threadPoolJob {
    ProgramExecutor& pe;
    threadPoolJob( ProgramExecutor& pe ): pe(pe) {}
    void run();
};

struct ProgramExecutor {
   public:
    std::vector<Op> _ops;
    unsigned short op_idx = 0;
    ulong instr_trigger = 0;
    bool has_executed = 0;
    ulong const& instr_clock;
    ulong max_instruction_per_tick = 0;

    ProgramExecutor(ulong const& instr_clock, ulong max_instruction_per_tick, ThreadPool<threadPoolJob>& threadPool);
    ProgramExecutor(Unpacker& p, ulong const& instr_clock, ulong max_instruction_per_tick, ThreadPool<threadPoolJob>& threadPool);
    void reset();
    void execute_async();
    void execute();
    void execute_sync();
    bool is_sync();

    friend Packer& operator<<(Packer& p, ProgramExecutor const& obj);
};


