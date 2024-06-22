#include "hardware/op_def.hpp"
#include "entity/map.hpp"
#include "entity/entity_actions.hpp"

#include "spdlog/spdlog.h"

// NOP //////////////////////////////////////////
ushort NoOP::operator()() {
    SPDLOG_TRACE("NOP operation executed");
    return 1;
}

// LOAD CONSTANT TO REGISTER ////////////////////
LoadConstantOp::LoadConstantOp(cpu_word_size& reg, bool& zero_flag, cpu_word_size const value)
    : reg(reg), zero_flag(zero_flag), value(value) {
        SPDLOG_DEBUG("LoadConstantOp created");
    }
ushort LoadConstantOp::operator()() {
    SPDLOG_DEBUG("Executing LoadConstantOp");
    SPDLOG_TRACE("Writing value {} to register", value);
    reg = value;
    zero_flag = value == 0;
    return 0;
}

// MOVE /////////////////////////////////////////
MoveOp::MoveOp(Map& map, MapEntity& entity, schar dx, schar dy, ulong speed)
    : map(map), entity(entity), dx(dx), dy(dy), speed(speed) {
        SPDLOG_DEBUG("MoveOp created");
        SPDLOG_TRACE("dx: {}, dy: {} speed: {}", dx, dy, speed);
    }
ushort MoveOp::operator()() {
    SPDLOG_DEBUG("Executing MoveOp");
    map.move_entity(entity, dx, dy);
    SPDLOG_TRACE("Moving ant by dx: {}, dy: {}", dx, dy);
    return speed;
}

// DIG /////////////////////////////////////////
DigOp::DigOp(Map& map, MapEntity& entity, Inventory& inventory, schar dx, schar dy, ulong speed)
    : map(map), entity(entity), inventory(inventory), dx(dx), dy(dy), speed(speed) {
        SPDLOG_DEBUG("DigOp created");
        SPDLOG_TRACE("dx: {}, dy: {}", dx, dy);
    }
ushort DigOp::operator()() {
    SPDLOG_DEBUG("Executing DigOp");
    handle_dig(map, entity, inventory, dx, dy);
    SPDLOG_TRACE("Digging ant by dx: {}, dy: {}", dx, dy);
    return speed;
}

// COPY REGISTER TO REGISTER ////////////////////
CopyOp::CopyOp(cpu_word_size& reg_src, cpu_word_size& reg_dst, bool& zero_flag)
    : reg_src(reg_src),
      reg_dst(reg_dst),
      zero_flag(zero_flag) {
        SPDLOG_DEBUG("CopyOp created");
      }
ushort CopyOp::operator()() {
    SPDLOG_DEBUG("Executing CopyOp");
    reg_dst = reg_src;
    zero_flag = reg_src == 0;
    SPDLOG_TRACE("Copying register with result {}", reg_dst);
    return 0;
}

// ADD REGISTER TO REGISTER ////////////////////
AddOp::AddOp(cpu_word_size& reg_src, cpu_word_size& reg_dst, bool& zero_flag)
    : reg_src(reg_src),
      reg_dst(reg_dst),
      zero_flag(zero_flag) {
        SPDLOG_TRACE("AddOp created");
      }
ushort AddOp::operator()() {
    SPDLOG_DEBUG("Executing AddOp");
    reg_dst += reg_src;
    zero_flag = reg_dst == 0;
    SPDLOG_TRACE("Adding registers - result: {}", reg_dst);
    return 0;
}

// SUB REGISTER TO REGISTER ////////////////////
SubOp::SubOp(cpu_word_size& reg_src, cpu_word_size& reg_dst, bool& zero_flag)
    : reg_src(reg_src),
      reg_dst(reg_dst),
      zero_flag(zero_flag) {
        SPDLOG_DEBUG("SubOp created");
      }
ushort SubOp::operator()() {
    SPDLOG_DEBUG("Executing SubOp");
    reg_dst -= reg_src;
    zero_flag = reg_dst == 0;
    SPDLOG_TRACE("Subtracting registers - result: {}", reg_dst);
    return 0;
}

// INC REGISTER ////////////////////////////////
IncOp::IncOp(cpu_word_size& reg, bool& zero_flag)
    : reg(reg), zero_flag(zero_flag) {
        SPDLOG_DEBUG("IncOp created");
    }
ushort IncOp::operator()() {
    SPDLOG_DEBUG("Executing IncOp");
    ++reg;
    zero_flag = reg == 0;
    SPDLOG_TRACE("Incremented register - result: {} - zero flag: {}", reg, zero_flag);
    return 0;
}

// DEC REGISTER ////////////////////////////////
DecOp::DecOp(cpu_word_size& reg, bool& zero_flag)
    : reg(reg), zero_flag(zero_flag) {
        SPDLOG_DEBUG("DecOp created");
    }
ushort DecOp::operator()() {
    SPDLOG_DEBUG("Executing DecOp");
    --reg;
    zero_flag = reg == 0;
    SPDLOG_TRACE("Decremented register - result: {} - zero flag: {}", reg, zero_flag);
    return 0;
}

// JMP /////////////////////////////////////////
JmpOp::JmpOp(ushort& op_idx, ushort new_idx)
    : op_idx(op_idx), new_idx(new_idx - 1) {
        SPDLOG_DEBUG("JmpOp created - jumping to: {}", new_idx);
    }

ushort JmpOp::operator()() {
    SPDLOG_DEBUG("Executing JmpOp");
    op_idx = new_idx;
    SPDLOG_TRACE("Jumped to op_idx {}", op_idx);
    return 0;
}

// JNZ /////////////////////////////////////////
JnzOp::JnzOp(ushort& op_idx, ushort new_idx, bool const& zero_flag)
    : op_idx(op_idx), new_idx(new_idx - 1), zero_flag(zero_flag) {
        SPDLOG_DEBUG("JnzOp created - jumping to: {}", new_idx);
    }

ushort JnzOp::operator()() {
    SPDLOG_DEBUG("Executing JnzOp");
    if(zero_flag){
        SPDLOG_TRACE("Zero flag is set, not jumping");
        return 0;
    }
    SPDLOG_TRACE("Zero flag off - jumping to op_idx {}", new_idx);
    op_idx = new_idx;
    return 0;
}
