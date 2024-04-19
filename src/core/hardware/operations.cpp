#include "hardware/operations.hpp"

#include <string>

#include "app/globals.hpp"
#include "hardware/ant_interactor.hpp"
#include "spdlog/spdlog.h"

Operations::Operations() : _ops(), label_map(), op_idx(), jmp_set() { SPDLOG_DEBUG("Operations created"); }
void Operations::add_op(const std::function<void()>&& op) {
    _ops.push_back(op);
}
void Operations::add_label(std::pair<std::string, size_t>&& p) {
    SPDLOG_DEBUG("Adding label {} at index {}", p.first, p.second);
    label_map.insert(p);
}
void Operations::set_op_idx(const std::string& label) {
    SPDLOG_DEBUG("Setting op_idx to {}", label);
    op_idx = label_map[label];
}
void Operations::set_op_idx(size_t idx) {
    SPDLOG_DEBUG("Setting op_idx to {}", idx);
    op_idx = idx;
}
const std::function<void()>& Operations::operator[](size_t idx) {
    SPDLOG_TRACE("Getting operation at index {}", idx);
    return _ops[idx];
}
size_t Operations::size() { return _ops.size(); }

void Operations::handleClockPulse() {
    SPDLOG_TRACE("Handling clock pulse for operations");
    if(op_idx >= size()) {
        SPDLOG_TRACE("No more operations to execute");
        return;  // don't do anything if there aren't anymore instructions
    }

    SPDLOG_TRACE("Executing operation at index {}", op_idx);
    (*this)[op_idx]();  // execute operation

    if(jmp_set) {
        SPDLOG_TRACE("Jump set, not incrementing op_idx");
        jmp_set = false;
    } else {
        ++op_idx;
        SPDLOG_TRACE("Incrementing op_idx to {}", op_idx);
    }
    SPDLOG_TRACE("Clock pulse handled for operations");
}

ParserStatus::ParserStatus() : p_err(false), err_msg("") {
    SPDLOG_DEBUG("ParserStatus created");
}
ParserStatus::ParserStatus(bool p_err, std::string err_msg)
    : p_err(p_err), err_msg(err_msg) {
        SPDLOG_DEBUG("ParserStatus created with error message");
    }
void ParserStatus::error(const std::string& err_msg) {
    SPDLOG_ERROR("ParserStatus error: {}", err_msg);
    this->p_err = true;
    this->err_msg = err_msg;
}

// NOP //////////////////////////////////////////
void NOP::operator()() {
    SPDLOG_TRACE("NOP operation executed");
}

// LOAD CONSTANT TO REGISTER ////////////////////
LoadConstantOp::LoadConstantOp(AntInteractor& interactor, long register_idx,
                               cpu_word_size const value)
    : interactor(interactor), register_idx(register_idx), value(value) {
        SPDLOG_DEBUG("LoadConstantOp created");
        SPDLOG_TRACE("Register index: {}, value: {}", register_idx, value);
    }
void LoadConstantOp::operator()() {
    SPDLOG_DEBUG("Executing LoadConstantOp");
    SPDLOG_TRACE("Writing value {} to register {}", value, register_idx);
    interactor.write_register(register_idx, value);
    interactor.write_zero_flag(value == 0);
}

// COPY REGISTER TO REGISTER ////////////////////
CopyOp::CopyOp(AntInteractor& interactor, long reg_src_idx, long reg_dst_idx)
    : interactor(interactor),
      reg_src_idx(reg_src_idx),
      reg_dst_idx(reg_dst_idx) {
        SPDLOG_DEBUG("CopyOp created");
        SPDLOG_TRACE("Source register index: {}, destination register index: {}",
                     reg_src_idx, reg_dst_idx);
      }
void CopyOp::operator()() {
    SPDLOG_DEBUG("Executing CopyOp");
    cpu_word_size const value = interactor.read_register(reg_src_idx);
    interactor.write_register(reg_dst_idx, value);
    interactor.write_zero_flag(value == 0);
    SPDLOG_TRACE("Copying register {} to register {} with result {}", reg_src_idx, reg_dst_idx, value);
}

// ADD REGISTER TO REGISTER ////////////////////
AddOp::AddOp(AntInteractor& interactor, long reg_src_idx, long reg_dst_idx)
    : interactor(interactor),
      reg_src_idx(reg_src_idx),
      reg_dst_idx(reg_dst_idx) {
        SPDLOG_TRACE("AddOp created");
        SPDLOG_TRACE("Source register index: {}, destination register index: {}",
                     reg_src_idx, reg_dst_idx);
      }
void AddOp::operator()() {
    SPDLOG_DEBUG("Executing AddOp");
    cpu_word_size const a = interactor.read_register(reg_src_idx);
    cpu_word_size const b = interactor.read_register(reg_dst_idx);
    interactor.write_register(reg_dst_idx, a + b);
    interactor.write_zero_flag((a + b) == 0);
    SPDLOG_TRACE("Adding register {} to register {} with result {}", reg_src_idx, reg_dst_idx, a + b);
}

// SUB REGISTER TO REGISTER ////////////////////
SubOp::SubOp(AntInteractor& interactor, long reg_src_idx, long reg_dst_idx)
    : interactor(interactor),
      reg_src_idx(reg_src_idx),
      reg_dst_idx(reg_dst_idx) {
        SPDLOG_DEBUG("SubOp created");
        SPDLOG_TRACE("Source register index: {}, destination register index: {}",
                     reg_src_idx, reg_dst_idx);
      }
void SubOp::operator()() {
    SPDLOG_DEBUG("Executing SubOp");
    cpu_word_size const a = interactor.read_register(reg_src_idx);
    cpu_word_size const b = interactor.read_register(reg_dst_idx);
    interactor.write_register(reg_dst_idx, a - b);
    interactor.write_zero_flag((a - b) == 0);
    SPDLOG_TRACE("Subtracted register {} from register {} with result {}", reg_src_idx, reg_dst_idx, a - b);
}

// INC REGISTER ////////////////////////////////
IncOp::IncOp(AntInteractor& interactor, long register_idx)
    : interactor(interactor), register_idx(register_idx) {
        SPDLOG_DEBUG("IncOp created");
        SPDLOG_TRACE("Register index: {}", register_idx);
    }
void IncOp::operator()() {
    SPDLOG_DEBUG("Executing IncOp");
    cpu_word_size const value = interactor.read_register(register_idx) + 1;
    interactor.write_register(register_idx, value);
    interactor.write_zero_flag(value == 0);
    SPDLOG_TRACE("Incremented register {} to {}", register_idx, value);
}

// DEC REGISTER ////////////////////////////////
DecOp::DecOp(AntInteractor& interactor, long register_idx)
    : interactor(interactor), register_idx(register_idx) {
        SPDLOG_DEBUG("DecOp created");
        SPDLOG_TRACE("Register index: {}", register_idx);
    }
void DecOp::operator()() {
    SPDLOG_DEBUG("Executing DecOp");
    cpu_word_size const value = interactor.read_register(register_idx) - 1;
    interactor.write_register(register_idx, value);
    interactor.write_zero_flag(value == 0);
    SPDLOG_TRACE("Decremented register {} to {}", register_idx, value);
}

// MOVE /////////////////////////////////////////
MoveOp::MoveOp(AntInteractor& interactor, long dx, long dy)
    : interactor(interactor), dx(dx), dy(dy) {
        SPDLOG_DEBUG("MoveOp created");
        SPDLOG_TRACE("dx: {}, dy: {}", dx, dy);
    }
void MoveOp::operator()() {
    SPDLOG_DEBUG("Executing MoveOp");
    interactor.try_move(dx, dy);
    SPDLOG_TRACE("Moving ant by dx: {}, dy: {}", dx, dy);
}

// JMP /////////////////////////////////////////
JmpOp::JmpOp(std::string label, Operations& operations)
    : addr(new std::string(label)), type(LABEL), operations(operations) {
        SPDLOG_DEBUG("JmpOp created - label: {}", label);
    }
JmpOp::JmpOp(size_t op_idx, Operations& operations)
    : addr(op_idx), type(INDEX), operations(operations) {
        SPDLOG_DEBUG("JmpOp created - op_idx: {}", op_idx);
    }

void JmpOp::operator()() {
    SPDLOG_DEBUG("Executing JmpOp");
    operations.jmp_set = true;
    if(type == LABEL) {
        SPDLOG_TRACE("Setting op_idx to label {}", *addr.str_lbl);
        operations.set_op_idx(*addr.str_lbl);
    } else {
        SPDLOG_TRACE("Setting op_idx to index {}", addr.op_idx);
        operations.set_op_idx(addr.op_idx);
    }
    SPDLOG_TRACE("Jumped to op_idx {}", operations.op_idx);
}

// JNZ /////////////////////////////////////////
JnzOp::JnzOp(AntInteractor& interactor, std::string label,
             Operations& operations)
    : JmpOp(label, operations), interactor(interactor) {
        SPDLOG_DEBUG("JnzOp created - label: {}", label);
    }
JnzOp::JnzOp(AntInteractor& interactor, size_t op_idx, Operations& operations)
    : JmpOp(op_idx, operations), interactor(interactor) {
        SPDLOG_DEBUG("JnzOp created - op_idx: {}", op_idx);
    }

void JnzOp::operator()() {
    SPDLOG_DEBUG("Executing JnzOp");
    if(interactor.read_zero_flag()){
        SPDLOG_TRACE("Zero flag is set, not jumping");
        return;
    }
    JmpOp::operator()();
}
