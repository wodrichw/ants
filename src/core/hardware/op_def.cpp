#include "hardware/op_def.hpp"

// NOP //////////////////////////////////////////
bool NoOP::operator()() {
    SPDLOG_TRACE("NOP operation executed");
    return false;
}

// LOAD CONSTANT TO REGISTER ////////////////////
LoadConstantOp::LoadConstantOp(cpu_word_size& reg, bool& zero_flag, cpu_word_size const value)
    : reg(reg), zero_flag(zero_flag), value(value) {
        SPDLOG_DEBUG("LoadConstantOp created");
    }
bool LoadConstantOp::operator()() {
    SPDLOG_DEBUG("Executing LoadConstantOp");
    SPDLOG_TRACE("Writing value {} to register", value);
    reg = value;
    zero_flag = value == 0;
    return true;
}

// MOVE /////////////////////////////////////////
MoveOp::MoveOp(Map& map, MapEntity& entity, schar dx, schar dy)
    : map(map), entity(entity), dx(dx), dy(dy) {
        SPDLOG_DEBUG("MoveOp created");
        SPDLOG_TRACE("dx: {}, dy: {}", dx, dy);
    }
bool MoveOp::operator()() {
    SPDLOG_DEBUG("Executing MoveOp");
    map.move_entity(entity, dx, dy);
    SPDLOG_TRACE("Moving ant by dx: {}, dy: {}", dx, dy);
    return false;
}

// COPY REGISTER TO REGISTER ////////////////////
CopyOp::CopyOp(cpu_word_size& reg_src, cpu_word_size& reg_dst, bool& zero_flag)
    : reg_src(reg_src),
      reg_dst(reg_dst),
      zero_flag(zero_flag) {
        SPDLOG_DEBUG("CopyOp created");
      }
bool CopyOp::operator()() {
    SPDLOG_DEBUG("Executing CopyOp");
    reg_dst = reg_src;
    zero_flag = reg_src == 0;
    SPDLOG_TRACE("Copying register with result {}", reg_dst);
    return true;
}

// ADD REGISTER TO REGISTER ////////////////////
AddOp::AddOp(cpu_word_size& reg_src, cpu_word_size& reg_dst, bool& zero_flag)
    : reg_src(reg_src),
      reg_dst(reg_dst),
      zero_flag(zero_flag) {
        SPDLOG_TRACE("AddOp created");
      }
bool AddOp::operator()() {
    SPDLOG_DEBUG("Executing AddOp");
    reg_dst += reg_src;
    zero_flag = reg_dst == 0;
    SPDLOG_TRACE("Adding registers - result: {}", reg_dst);
    return true;
}

// SUB REGISTER TO REGISTER ////////////////////
SubOp::SubOp(cpu_word_size& reg_src, cpu_word_size& reg_dst, bool& zero_flag)
    : reg_src(reg_src),
      reg_dst(reg_dst),
      zero_flag(zero_flag) {
        SPDLOG_DEBUG("SubOp created");
      }
bool SubOp::operator()() {
    SPDLOG_DEBUG("Executing SubOp");
    reg_dst -= reg_src;
    zero_flag = reg_dst == 0;
    SPDLOG_TRACE("Subtracting registers - result: {}", reg_dst);
    return true;
}

// INC REGISTER ////////////////////////////////
IncOp::IncOp(cpu_word_size& reg, bool& zero_flag)
    : reg(reg), zero_flag(zero_flag) {
        SPDLOG_DEBUG("IncOp created");
    }
bool IncOp::operator()() {
    SPDLOG_DEBUG("Executing IncOp");
    ++reg;
    zero_flag = reg == 0;
    SPDLOG_TRACE("Incremented register - result: {} - zero flag: %d", reg, zero_flag);
    return true;
}

// DEC REGISTER ////////////////////////////////
DecOp::DecOp(cpu_word_size& reg, bool& zero_flag)
    : reg(reg), zero_flag(zero_flag) {
        SPDLOG_DEBUG("DecOp created");
    }
bool DecOp::operator()() {
    SPDLOG_DEBUG("Executing DecOp");
    --reg;
    zero_flag = reg == 0;
    SPDLOG_TRACE("Decremented register - result: {} - zero flag: %d", reg, zero_flag);
    return true;
}

// JMP /////////////////////////////////////////
JmpOp::JmpOp(ushort& op_idx, ushort new_idx)
    : op_idx(op_idx), new_idx(new_idx - 1) {
        SPDLOG_DEBUG("JmpOp created - jumping to: {}", new_idx);
    }

bool JmpOp::operator()() {
    SPDLOG_DEBUG("Executing JmpOp");
    op_idx = new_idx;
    SPDLOG_TRACE("Jumped to op_idx {}", op_idx);
    return true;
}

// JNZ /////////////////////////////////////////
JnzOp::JnzOp(ushort& op_idx, ushort new_idx, bool const& zero_flag)
    : op_idx(op_idx), new_idx(new_idx - 1), zero_flag(zero_flag) {
        SPDLOG_DEBUG("JnzOp created - jumping to: {}", new_idx);
    }

bool JnzOp::operator()() {
    SPDLOG_DEBUG("Executing JnzOp");
    if(zero_flag){
        SPDLOG_TRACE("Zero flag is set, not jumping");
        return true;
    }
    SPDLOG_TRACE("Zero flag off - jumping to op_idx {}", new_idx);
    op_idx = new_idx;
    return true;
}
