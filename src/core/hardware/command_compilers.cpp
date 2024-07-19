#include "hardware/command_compilers.hpp"
#include "hardware/compile_args.hpp"
#include "hardware/op_def.hpp"
#include "hardware/program_executor.hpp"
#include "hardware/brain.hpp"
#include "utils/status.hpp"

#include "spdlog/spdlog.h"

using uchar = unsigned char;
using schar = signed char;

void NOP_Compiler::operator()(CompileArgs &args) {
    SPDLOG_TRACE("Compiling NOP command");
    args.interactor.ops.push_back({
        1,
        NoOP()
    });
    SPDLOG_TRACE("NOP command compiled");
}

void LoadConstantCompiler::operator()(CompileArgs &args) {
    SPDLOG_TRACE("Compiling Load Constant command");
    AntInteractor &interactor = args.interactor;
    char const register_idx = ((*(args.code_it++)) & 1);
    
    uchar const v0 = *(args.code_it++);
    uchar const v1 = *(args.code_it++);
    uchar const v2 = *(args.code_it++);
    uchar const v3 = *(args.code_it++);
    
    cpu_word_size const value = v0 | (v1 << 8) | (v2 << 16) << (v3 << 24);

    interactor.ops.push_back({
        0,
        LoadConstantOp(interactor.registers[register_idx], interactor.registers.zero_flag, value)
    });
    SPDLOG_TRACE("Load Constant command compiled");
}

void CopyCompiler::operator()(CompileArgs &args) {
    SPDLOG_TRACE("Compiling Copy command");
    AntInteractor &interactor = args.interactor;
    uchar const register_names = *(args.code_it++);
    uchar const reg_src_idx = ((register_names >> 1) & 1);
    uchar const reg_dst_idx = (register_names & 1);
    interactor.ops.push_back({
        0,
        CopyOp(interactor.registers[reg_src_idx], interactor.registers[reg_dst_idx],
               interactor.registers.zero_flag)
    });
    
    SPDLOG_TRACE("Copy command compiled");
}

void AddCompiler::operator()(CompileArgs &args) {
    SPDLOG_TRACE("Compiling Add command");
    AntInteractor &interactor = args.interactor;
    uchar const register_names = *(args.code_it++);
    uchar const reg_src_idx = ((register_names >> 1) & 1);
    uchar const reg_dst_idx = register_names & 1;
    interactor.ops.push_back({
        0,
        AddOp(interactor.registers[reg_src_idx], interactor.registers[reg_dst_idx],
              interactor.registers.zero_flag)
    });
    
    SPDLOG_TRACE("Add command compiled");
}

void SubCompiler::operator()(CompileArgs &args) {
    SPDLOG_TRACE("Compiling Sub command");
    AntInteractor &interactor = args.interactor;
    uchar const register_names = *(args.code_it++);
    uchar const reg_src_idx = ((register_names >> 1) & 1);
    uchar const reg_dst_idx = register_names & 1;
    interactor.ops.push_back({
        0,
        SubOp(interactor.registers[reg_src_idx], interactor.registers[reg_dst_idx],
              interactor.registers.zero_flag)
    });
    
    SPDLOG_TRACE("Sub command compiled");
}

void IncCompiler::operator()(CompileArgs &args) {
    SPDLOG_TRACE("Compiling Increment command");
    AntInteractor &interactor = args.interactor;
    uchar const register_idx = (*(args.code_it++)) & 1;
    interactor.ops.push_back({
        0,
        IncOp(interactor.registers[register_idx], interactor.registers.zero_flag)
    });
    
    SPDLOG_TRACE("Increment command compiled");
}

void DecCompiler::operator()(CompileArgs &args) {
    SPDLOG_TRACE("Compiling Decrement command");
    AntInteractor &interactor = args.interactor;
    uchar const register_idx = ((*(args.code_it++)) & 1);
    interactor.ops.push_back({
        0,
        DecOp(interactor.registers[register_idx], interactor.registers.zero_flag)
    });
    
    SPDLOG_TRACE("Decrement command compiled");
}

void MoveAntCompiler::operator()(CompileArgs &args) {
    SPDLOG_TRACE("Compiling Move EntityData command");
    AntInteractor &interactor = args.interactor;
    uchar movement = (*(args.code_it++)) & 0b111;
    schar dx = (movement & 1) * (movement - 2), dy = (1 - (movement & 1)) * (movement - 1);

    interactor.ops.push_back({
        interactor.move_speed,
        MoveOp(interactor.map, interactor.entity, dx, dy, interactor.move_speed)
    });
    
    SPDLOG_TRACE("Move EntityData command compiled, move speed {}", interactor.move_speed);
}

void DigAntCompiler::operator()(CompileArgs &args) {
    SPDLOG_TRACE("Compiling Move EntityData command");
    AntInteractor &interactor = args.interactor;
    uchar movement = (*(args.code_it++)) & 0b111;
    schar dx = (movement & 1) * (movement - 2), dy = (1 - (movement & 1)) * (movement - 1);

    interactor.ops.push_back({
        interactor.move_speed,
        DigOp(interactor.map, interactor.entity, interactor.inventory, dx, dy, interactor.move_speed)
    });
    
    SPDLOG_TRACE("Move EntityData command compiled, move_speed {}", interactor.move_speed);
}

void JumpCompiler::operator()(CompileArgs &args) {
    SPDLOG_TRACE("Compiling Jump command");
    AntInteractor &interactor = args.interactor;
    ++args.code_it;
    ushort lower_half = *(args.code_it++);
    ushort upper_half = *(args.code_it++);
    ushort const label_idx = lower_half | (upper_half << 8);
    interactor.ops.push_back({
        0,
        JmpOp(interactor.op_idx, label_idx)
    });

    SPDLOG_TRACE("Jump command compiled");
}

void JumpNotZeroCompiler::operator()(CompileArgs &args) {
    SPDLOG_TRACE("Compiling Jump Not Zero command");
    AntInteractor &interactor = args.interactor;

    ++args.code_it;
    ushort lower_half = *(args.code_it++);
    ushort upper_half = *(args.code_it++);
    ushort const label_idx = lower_half | (upper_half << 8);
    interactor.ops.push_back({
        0,
        JnzOp(interactor.op_idx, label_idx, interactor.registers.zero_flag)
    });
    
    SPDLOG_TRACE("Jump Not Zero command compiled");
}
