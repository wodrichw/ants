#include <string>

#include "operations.hpp"

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

ParserStatus::ParserStatus() : p_err(false), err_msg("") {}
ParserStatus::ParserStatus(bool p_err, std::string err_msg)
    : p_err(p_err), err_msg(err_msg) {}
void ParserStatus::error(const std::string &err_msg)
{
  this->p_err = true;
  this->err_msg = err_msg;
}

EngineInteractor::EngineInteractor(const EngineInteractor& rhs):
    move_ant(rhs.move_ant), status()
{
    status.p_err = rhs.status.p_err;
    status.err_msg = rhs.status.err_msg;
}

// NOP
void NOP::operator()() {}

// LOAD CONSTANT TO REGISTER
LoadConstantOp::LoadConstantOp(cpu_word_size &reg, cpu_word_size const value)
    : reg(reg), value(value) {}
void LoadConstantOp::operator()() { reg = value; }

// MOVE /////////////////////////////////////////
MoveOp::MoveOp(const EngineInteractor& interactor, int dx, int dy): interactor(interactor), dx(dx), dy(dy) {}
void MoveOp::operator()() { interactor.move_ant(dx, dy); }

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
