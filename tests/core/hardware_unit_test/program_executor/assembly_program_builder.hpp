#pragma once

#include <vector>
#include <functional>
#include "hardware/program_executor.hpp"
#include "spdlog/spdlog.h"

class AssemblyProgramBuilder {
public:
    AssemblyProgramBuilder() = default;

    // NOP Instruction (0b00000) - Async, 0 ticks
    AssemblyProgramBuilder& addNOP() {
        SPDLOG_TRACE("Adding NOP instruction");
        ops.emplace_back();
        ops.back().num_ticks = 0;  // Async
        ops.back().fn = []() {
            SPDLOG_TRACE("Executing NOP - no operation");
        };
        return *this;
    }

    // MOVE Instruction (0b00001) - Sync, 1 tick
    AssemblyProgramBuilder& addMOVE() {
        SPDLOG_TRACE("Adding MOVE instruction");
        ops.emplace_back();
        ops.back().num_ticks = 1;  // Sync - interacts with map/other ants
        ops.back().fn = []() {
            SPDLOG_TRACE("Executing MOVE - synchronous movement");
        };
        return *this;
    }

    // LOAD Constant Instruction (0b00010) - Async, 0 ticks
    AssemblyProgramBuilder& addLOAD(uint32_t constant, bool register_a = true) {
        SPDLOG_TRACE("Adding LOAD instruction - constant: {}, register_a: {}", constant, register_a);
        ops.emplace_back();
        ops.back().num_ticks = 0;  // Async - only affects local registers
        ops.back().fn = [constant, register_a]() {
            SPDLOG_TRACE("Executing LOAD - loading {} into register {}",
                        constant, register_a ? "A" : "B");
        };
        return *this;
    }

    // COPY Instruction (0b00011) - Async, 0 ticks
    AssemblyProgramBuilder& addCOPY(bool from_a_to_b = true) {
        SPDLOG_TRACE("Adding COPY instruction - from_a_to_b: {}", from_a_to_b);
        ops.emplace_back();
        ops.back().num_ticks = 0;  // Async - only affects local registers
        ops.back().fn = [from_a_to_b]() {
            SPDLOG_TRACE("Executing COPY - copying from {} to {}",
                        from_a_to_b ? "A" : "B", from_a_to_b ? "B" : "A");
        };
        return *this;
    }

    // ADD Instruction (0b00100) - Async, 0 ticks
    AssemblyProgramBuilder& addADD(bool register_a = true) {
        SPDLOG_TRACE("Adding ADD instruction - register_a: {}", register_a);
        ops.emplace_back();
        ops.back().num_ticks = 0;  // Async - only affects local registers
        ops.back().fn = [register_a]() {
            SPDLOG_TRACE("Executing ADD - adding to register {}", register_a ? "A" : "B");
        };
        return *this;
    }

    // INC Instruction (0b00110) - Async, 0 ticks
    AssemblyProgramBuilder& addINC(bool register_a = true) {
        SPDLOG_TRACE("Adding INC instruction - register_a: {}", register_a);
        ops.emplace_back();
        ops.back().num_ticks = 0;  // Async - only affects local registers
        ops.back().fn = [register_a]() {
            SPDLOG_TRACE("Executing INC - incrementing register {}", register_a ? "A" : "B");
        };
        return *this;
    }

    // JMP Instruction (0b01010) - Async, 0 ticks
    AssemblyProgramBuilder& addJMP(uint16_t address) {
        SPDLOG_TRACE("Adding JMP instruction - address: {}", address);
        ops.emplace_back();
        ops.back().num_ticks = 0;  // Async - only affects local instruction pointer
        ops.back().fn = [address]() {
            SPDLOG_TRACE("Executing JMP - jumping to address {}", address);
        };
        return *this;
    }

    // DIG Instruction (0b10001) - Sync, 2 ticks (interacts with map)
    AssemblyProgramBuilder& addDIG() {
        SPDLOG_TRACE("Adding DIG instruction");
        ops.emplace_back();
        ops.back().num_ticks = 2;  // Sync - interacts with map
        ops.back().fn = []() {
            SPDLOG_TRACE("Executing DIG - synchronous digging operation");
        };
        return *this;
    }

    // CHK Instruction (0b10010) - Sync, 1 tick (checks map state)
    AssemblyProgramBuilder& addCHK() {
        SPDLOG_TRACE("Adding CHK instruction");
        ops.emplace_back();
        ops.back().num_ticks = 1;  // Sync - checks map state
        ops.back().fn = []() {
            SPDLOG_TRACE("Executing CHK - synchronous check operation");
        };
        return *this;
    }

    // RT (Turn Right) Instruction (0b10000) - Async, 0 ticks
    AssemblyProgramBuilder& addRT() {
        SPDLOG_TRACE("Adding RT (turn right) instruction");
        ops.emplace_back();
        ops.back().num_ticks = 0;  // Async - only affects local state
        ops.back().fn = []() {
            SPDLOG_TRACE("Executing RT - turning right");
        };
        return *this;
    }

    // LT (Turn Left) Instruction (0b01111) - Async, 0 ticks
    AssemblyProgramBuilder& addLT() {
        SPDLOG_TRACE("Adding LT (turn left) instruction");
        ops.emplace_back();
        ops.back().num_ticks = 0;  // Async - only affects local state
        ops.back().fn = []() {
            SPDLOG_TRACE("Executing LT - turning left");
        };
        return *this;
    }

    std::vector<Op> build() {
        SPDLOG_DEBUG("Built assembly program with {} instructions", ops.size());
        return std::move(ops);
    }

    void clear() {
        ops.clear();
    }

private:
    std::vector<Op> ops;
};
