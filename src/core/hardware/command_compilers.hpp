#pragma once

#include "hardware/brain.hpp"
#include "hardware/command_config.hpp"
#include "hardware/compile_args.hpp"
#include "hardware/op_def.hpp"
#include "hardware/program_executor.hpp"
#include "spdlog/spdlog.h"

using uchar = unsigned char;
using schar = signed char;
using ushort = unsigned short;

template <typename Operation, unsigned short TickCount = 0>
struct NoArgCommandCompiler {
    void operator()(CommandConfig const& config, CompileArgs& args) {
        SPDLOG_TRACE("Compiling {} command", config.command_string);
        args.ops.push_back({TickCount, Operation(args.cpu)});
        ++args.code_it;
        SPDLOG_TRACE("{} command compiled", config.command_string);
        (void)config;
    }
};

template <typename Operation, unsigned short TickCount = 0>
struct LoadConstantCompiler {
    void operator()(CommandConfig const& config, CompileArgs& args) {
        SPDLOG_TRACE("Compiling {} command", config.command_string);
        char const register_idx = ((*args.code_it) & 1);

        cpu_word_size const v0 = *(++args.code_it);
        cpu_word_size const v1 = *(++args.code_it);
        cpu_word_size const v2 = *(++args.code_it);
        cpu_word_size const v3 = *(++args.code_it);

        cpu_word_size const value = v0 | (v1 << 8) | (v2 << 16) << (v3 << 24);

        args.ops.push_back(
            {TickCount, Operation(args.cpu, args.cpu[register_idx], value)});

        ++args.code_it;
        SPDLOG_TRACE("{} command compiled", config.command_string);
        (void)config;
    }
};

template <typename Operation, unsigned short TickCount = 0>
struct TwoRegisterCommandCompiler {
    void operator()(CommandConfig const& config, CompileArgs& args) {
        SPDLOG_TRACE("Compiling {} command", config.command_string);
        uchar const register_names = *args.code_it;
        uchar const reg_src_idx = ((register_names >> 1) & 1);
        uchar const reg_dst_idx = (register_names & 1);
        args.ops.push_back(
            {TickCount, Operation(args.cpu, args.cpu[reg_src_idx],
                                  args.cpu[reg_dst_idx])});

        ++args.code_it;
        SPDLOG_TRACE("{} command compiled", config.command_string);
        (void)config;
    }
};

template <typename Operation, unsigned short TickCount = 0>
struct OneRegisterCommandCompiler {
    void operator()(CommandConfig const& config, CompileArgs& args) {
        SPDLOG_TRACE("Compiling {} command", config.command_string);
        uchar const register_idx = (*args.code_it) & 1;
        args.ops.push_back(
            {TickCount, Operation(args.cpu, args.cpu[register_idx])});

        ++args.code_it;
        SPDLOG_TRACE("{} command compiled", config.command_string);
        (void)config;
    }
};

template <typename Operation>
struct MoveAntCompiler {
    void operator()(CommandConfig const& config, CompileArgs& args) {
        SPDLOG_TRACE("Compiling {} command - speed: {} tks / dig",
                     config.command_string, args.cpu.wait_move_tick_count);
        uchar const instruction = *args.code_it;
        bool const has_direction = (instruction & 0b100) != 0;
        bool const dir_flag1 = (instruction & 0b010) != 0;
        bool const dir_flag2 = (instruction & 0b001) != 0;

        if(has_direction) {
            DualRegisters& cpu = args.cpu;
            args.ops.push_back({
                args.cpu.wait_move_tick_count,
                [dir_flag1, dir_flag2, &cpu]() {
                    cpu.dir_flag1 = dir_flag1;
                    cpu.dir_flag2 = dir_flag2;
                    Operation op(cpu);
                    op();
                }});
        } else {
            args.ops.push_back(
                {args.cpu.wait_move_tick_count, Operation(args.cpu)});
        }

        ++args.code_it;
        SPDLOG_TRACE("{} command compiled - speed: {} tks / move",
                     config.command_string, args.cpu.wait_move_tick_count);
        (void)config;
    }
};

template <typename Operation>
struct DigAntCompiler {
    void operator()(CommandConfig const& config, CompileArgs& args) {
        SPDLOG_TRACE("Compiling {} command", config.command_string);

        args.ops.push_back({args.cpu.wait_dig_tick_count, DigOp(args.cpu)});

        ++args.code_it;
        SPDLOG_TRACE("{} command compiled - speed: {} tks / dig",
                     config.command_string, args.cpu.wait_dig_tick_count);
        (void)config;
    }
};

template <typename Operation, unsigned short TickCount = 0>
struct JumpCompiler {
    void operator()(CommandConfig const& config, CompileArgs& args) {
        SPDLOG_TRACE("Compiling {} command", config.command_string);
        ushort lower_half = *(++args.code_it);
        ushort upper_half = *(++args.code_it);
        ushort const address = lower_half | (upper_half << 8);
        args.ops.push_back({TickCount, Operation(args.cpu, address)});

        ++args.code_it;
        SPDLOG_TRACE("{} command compiled", config.command_string);
        (void)config;
    }
};

template <typename Operation, unsigned short TickCount = 0>
struct OneScentCommandCompiler {
    void operator()(CommandConfig const& config, CompileArgs& args) {
        SPDLOG_TRACE("Compiling {} command", config.command_string);
        uchar const scent_idx = (*args.code_it) & 0b111;
        args.ops.push_back({TickCount, Operation(args.cpu, scent_idx)});

        ++args.code_it;
        SPDLOG_TRACE("{} command compiled", config.command_string);
        (void)config;
    }
};

template <typename Operation, unsigned short TickCount = 0>
struct SetScentPriorityCompiler {
    void operator()(CommandConfig const& config, CompileArgs& args) {
        SPDLOG_TRACE("Compiling {} command", config.command_string);
        uchar const scent_idx = (*args.code_it) & 0b111;
        ++args.code_it;

        uchar const priority = (*args.code_it);
        args.ops.push_back(
            {TickCount, Operation(args.cpu, scent_idx, priority)});

        ++args.code_it;
        SPDLOG_TRACE("{} command compiled", config.command_string);
        (void)config;
    }
};
