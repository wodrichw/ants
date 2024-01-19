#include <string>

#include "operations.hpp"
#include "ant_interactor.hpp"
#include "globals.hpp"

Operations::Operations() : _ops(), label_map(), op_idx(), jmp_set() {}
void Operations::add_op(const std::function<void()> &&op)
{
    _ops.push_back(op);
}
void Operations::add_label(std::pair<std::string, size_t> &&p)
{
    label_map.insert(p);
}
void Operations::set_op_idx(const std::string &label)
{
    op_idx = label_map[label];
}
void Operations::set_op_idx(size_t idx) { op_idx = idx; }
const std::function<void()> &Operations::operator[](size_t idx)
{
    return _ops[idx];
}
size_t Operations::size() { return _ops.size(); }

void Operations::handleClockPulse()
{
    if( op_idx >= size() ) return; // don't do anything if there aren't anymore instructions

    (*this)[op_idx](); // execute operation

    if( jmp_set ) {
        jmp_set = false;
    } else {
        ++op_idx;
    }
}

ParserStatus::ParserStatus() : p_err(false), err_msg("") {}
ParserStatus::ParserStatus(bool p_err, std::string err_msg)
    : p_err(p_err), err_msg(err_msg) {}
void ParserStatus::error(const std::string &err_msg)
{
    this->p_err = true;
    this->err_msg = err_msg;
}

// NOP //////////////////////////////////////////
void NOP::operator()() {}

// LOAD CONSTANT TO REGISTER ////////////////////
LoadConstantOp::LoadConstantOp(AntInteractor& interactor, long register_idx, cpu_word_size const value)
    : interactor(interactor), register_idx(register_idx), value(value) {}
void LoadConstantOp::operator()() {
    interactor.write_register(register_idx, value);
    interactor.write_zero_flag(value == 0);
}

// COPY REGISTER TO REGISTER ////////////////////
CopyOp::CopyOp(AntInteractor& interactor, long reg_src_idx, long reg_dst_idx)
    : interactor(interactor), reg_src_idx(reg_src_idx), reg_dst_idx(reg_dst_idx) {}
void CopyOp::operator()() {
    cpu_word_size const value = interactor.read_register(reg_src_idx);
    interactor.write_register(reg_dst_idx, value);
    interactor.write_zero_flag(value == 0);
}

// ADD REGISTER TO REGISTER ////////////////////
AddOp::AddOp(AntInteractor& interactor, long reg_src_idx, long reg_dst_idx)
    : interactor(interactor), reg_src_idx(reg_src_idx), reg_dst_idx(reg_dst_idx) {}
void AddOp::operator()() {
  cpu_word_size const a = interactor.read_register(reg_src_idx);
  cpu_word_size const b = interactor.read_register(reg_dst_idx);
  interactor.write_register(reg_dst_idx, a + b);
    interactor.write_zero_flag((a + b) == 0);
}

// SUB REGISTER TO REGISTER ////////////////////
SubOp::SubOp(AntInteractor& interactor, long reg_src_idx, long reg_dst_idx)
    : interactor(interactor), reg_src_idx(reg_src_idx), reg_dst_idx(reg_dst_idx) {}
void SubOp::operator()() {
    cpu_word_size const a = interactor.read_register(reg_src_idx);
    cpu_word_size const b = interactor.read_register(reg_dst_idx);
    interactor.write_register(reg_dst_idx, a - b);
    interactor.write_zero_flag((a - b) == 0);
}

// INC REGISTER ////////////////////////////////
IncOp::IncOp(AntInteractor& interactor, long register_idx)
    : interactor(interactor), register_idx(register_idx) {}
void IncOp::operator()() {
    cpu_word_size const value = interactor.read_register(register_idx) + 1;
    interactor.write_register(register_idx, value);
    interactor.write_zero_flag(value == 0);
}

// DEC REGISTER ////////////////////////////////
DecOp::DecOp(AntInteractor& interactor, long register_idx)
    : interactor(interactor), register_idx(register_idx) {}
void DecOp::operator()() {
    cpu_word_size const value = interactor.read_register(register_idx) - 1;
    interactor.write_register(register_idx, value);
    interactor.write_zero_flag(value == 0);
}

// MOVE /////////////////////////////////////////
MoveOp::MoveOp(AntInteractor& interactor, long dx, long dy): interactor(interactor), dx(dx), dy(dy) {}
void MoveOp::operator()() { interactor.try_move(dx, dy); }

// JMP /////////////////////////////////////////
JmpOp::JmpOp(std::string label, Operations& operations):  addr(new std::string(label)), type(LABEL), operations(operations) {}
JmpOp::JmpOp(size_t op_idx, Operations& operations):  addr(op_idx), type(INDEX), operations(operations) {}

void JmpOp::operator()()
{
    operations.jmp_set = true;
    if( type == LABEL ) {
        operations.set_op_idx(*addr.str_lbl);
    } else {
        operations.set_op_idx(addr.op_idx);
    }
}

// JNZ /////////////////////////////////////////
JnzOp::JnzOp(AntInteractor& interactor, std::string label, Operations& operations):  JmpOp(label, operations), interactor(interactor) {}
JnzOp::JnzOp(AntInteractor& interactor, size_t op_idx, Operations& operations):  JmpOp(op_idx, operations), interactor(interactor) {}

void JnzOp::operator()()
{
    if (interactor.read_zero_flag()) return;
    JmpOp::operator()();
}
