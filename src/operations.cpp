#include "operations.hpp"

// MOVE /////////////////////////////////////////
MoveOp::MoveOp(const EngineInteractor& interactor, int dx, int dy): interactor(interactor), dx(dx), dy(dy) {}

void MoveOp::operator()()
{
    interactor.move_ant(dx, dy);
}

// GOTO /////////////////////////////////////////
GotoOp::GotoOp(std::string label, Operations& operations):  addr(new std::string(label)), type(LABEL), operations(operations) {}
GotoOp::GotoOp(size_t op_idx, Operations& operations):  addr(op_idx), type(INDEX), operations(operations) {}

void GotoOp::operator()()
{
    operations.goto_set = true;
    if( type == LABEL ) {
        operations.set_op_idx(*addr.str_lbl);
    } else {
        operations.set_op_idx(addr.op_idx);
    }
}

