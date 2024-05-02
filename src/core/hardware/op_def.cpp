#include "hardware/op_def.hpp"

// NOP //////////////////////////////////////////
void NOP::operator()() {
    SPDLOG_TRACE("NOP operation executed");
}

// LOAD CONSTANT TO REGISTER ////////////////////
LoadConstantOp::LoadConstantOp(cpu_word_size& reg, bool& zero_flag, cpu_word_size const value)
    : reg(reg), zero_flag(zero_flag), value(value) {
        SPDLOG_DEBUG("LoadConstantOp created");
    }
void LoadConstantOp::operator()() {
    SPDLOG_DEBUG("Executing LoadConstantOp");
    SPDLOG_TRACE("Writing value {} to register", value);
    reg = value;
    zero_flag = value == 0;
}

// MOVE /////////////////////////////////////////
MoveOp::MoveOp(Map& map, MapEntity& entity, long dx, long dy)
    : map(map), entity(entity), dx(dx), dy(dy) {
        SPDLOG_DEBUG("MoveOp created");
        SPDLOG_TRACE("dx: {}, dy: {}", dx, dy);
    }
void MoveOp::operator()() {
    SPDLOG_DEBUG("Executing MoveOp");
    map.move_entity(entity, dx, dy);
    SPDLOG_TRACE("Moving ant by dx: {}, dy: {}", dx, dy);
}

// COPY REGISTER TO REGISTER ////////////////////
CopyOp::CopyOp(cpu_word_size& reg_src, cpu_word_size& reg_dst, bool& zero_flag)
    : reg_src(reg_src),
      reg_dst(reg_dst),
      zero_flag(zero_flag) {
        SPDLOG_DEBUG("CopyOp created");
      }
void CopyOp::operator()() {
    SPDLOG_DEBUG("Executing CopyOp");
    reg_dst = reg_src;
    zero_flag = reg_src == 0;
    SPDLOG_TRACE("Copying register with result {}", reg_dst);
}

// ADD REGISTER TO REGISTER ////////////////////
AddOp::AddOp(cpu_word_size& reg_src, cpu_word_size& reg_dst, bool& zero_flag)
    : reg_src(reg_src),
      reg_dst(reg_dst),
      zero_flag(zero_flag) {
        SPDLOG_TRACE("AddOp created");
      }
void AddOp::operator()() {
    SPDLOG_DEBUG("Executing AddOp");
    reg_dst += reg_src;
    zero_flag = reg_dst == 0;
    SPDLOG_TRACE("Adding registers - result: {}", reg_dst);
}

// SUB REGISTER TO REGISTER ////////////////////
SubOp::SubOp(cpu_word_size& reg_src, cpu_word_size& reg_dst, bool& zero_flag)
    : reg_src(reg_src),
      reg_dst(reg_dst),
      zero_flag(zero_flag) {
        SPDLOG_DEBUG("SubOp created");
      }
void SubOp::operator()() {
    SPDLOG_DEBUG("Executing SubOp");
    reg_dst -= reg_src;
    zero_flag = reg_dst == 0;
    SPDLOG_TRACE("Subtracting registers - result: {}", reg_dst);
}

// INC REGISTER ////////////////////////////////
IncOp::IncOp(cpu_word_size& reg, bool& zero_flag)
    : reg(reg), zero_flag(zero_flag) {
        SPDLOG_DEBUG("IncOp created");
    }
void IncOp::operator()() {
    SPDLOG_DEBUG("Executing IncOp");
    ++reg;
    zero_flag = reg == 0;
    SPDLOG_TRACE("Incremented register - result: {}", reg);
}

// DEC REGISTER ////////////////////////////////
DecOp::DecOp(cpu_word_size& reg, bool& zero_flag)
    : reg(reg), zero_flag(zero_flag) {
        SPDLOG_DEBUG("DecOp created");
    }
void DecOp::operator()() {
    SPDLOG_DEBUG("Executing DecOp");
    --reg;
    zero_flag = reg == 0;
    SPDLOG_TRACE("Decremented register - result: {}", reg);
}

// JMP /////////////////////////////////////////
JmpOp::JmpOp(size_t& op_idx, size_t new_idx)
    : op_idx(op_idx), new_idx(new_idx - 1) {
        SPDLOG_DEBUG("JmpOp created - jumping to: {}", new_idx);
    }

void JmpOp::operator()() {
    SPDLOG_DEBUG("Executing JmpOp");
    op_idx = new_idx;
    SPDLOG_TRACE("Jumped to op_idx {}", op_idx);
}

// JNZ /////////////////////////////////////////
JnzOp::JnzOp(size_t& op_idx, size_t new_idx, bool const& zero_flag)
    : op_idx(op_idx), new_idx(new_idx), zero_flag(zero_flag) {
        SPDLOG_DEBUG("JnzOp created - jumping to: {}", new_idx);
    }

void JnzOp::operator()() {
    SPDLOG_DEBUG("Executing JnzOp");
    if(zero_flag){
        SPDLOG_TRACE("Zero flag is set, not jumping");
        return;
    }
    SPDLOG_TRACE("Zero flag off - jumping to op_idx {}", op_idx);
    op_idx = new_idx;
}
