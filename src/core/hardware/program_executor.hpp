#pragma once

#include <functional>
#include <vector>

#include "hardware.pb.h"
#include "utils/thread_pool.hpp"
#include "hardware/op_def.hpp"

using ulong = unsigned long;
using ushort = unsigned short;

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
    std::vector<Op> _ops = {};
    ushort& instr_ptr_register;
    ulong instr_trigger = 0;
    bool has_executed_async = false;
    bool has_executed_sync = false;
    ulong const& instr_clock;
    ulong max_instruction_per_tick = 0;
    ThreadPool<AsyncProgramJob>& job_pool;

    ProgramExecutor(ulong const& instr_clock, ulong max_instruction_per_tick, ushort& instr_ptr_register, ThreadPool<AsyncProgramJob>&);
    ProgramExecutor(Unpacker& p, ulong const& instr_clock, ulong max_instruction_per_tick, ushort& instr_ptr_register, ThreadPool<AsyncProgramJob>&);
    ProgramExecutor(const ant_proto::ProgramExecutor& msg, ulong const& instr_clock, ulong max_instruction_per_tick, ushort& instr_ptr_register, ThreadPool<AsyncProgramJob>&);
    void reset();
    void execute_async();
    void execute();
    void execute_sync();
    bool is_sync();

    ant_proto::ProgramExecutor get_proto();
    friend Packer& operator<<(Packer& p, ProgramExecutor const& obj);
};


