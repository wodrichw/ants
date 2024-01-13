#include <string>

#include "operations.hpp"

// MOVE /////////////////////////////////////////
MoveOp::MoveOp(const EngineInteractor& interactor, int dx, int dy): interactor(interactor), dx(dx), dy(dy) {}

void MoveOp::operator()()
{
    interactor.move_ant(dx, dy);
}

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

