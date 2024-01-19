#pragma once 

#include <functional>
#include <unordered_map>
#include <vector>
#include <string>

#include "globals.hpp"
#include "ant_interactor.hpp"

class Operations {
    std::vector<std::function<void()>> _ops;
    std::unordered_map<std::string, size_t> label_map;
public:
    size_t op_idx;
    bool jmp_set;
    Operations();

    void add_op(const std::function<void()> &&op);
    void add_label(std::pair<std::string, size_t> &&p);

    void set_op_idx(const std::string &label);
    void set_op_idx(size_t idx);
    const std::function<void()> &operator[](size_t idx);
    size_t size();
    void handleClockPulse();
};

struct ParserStatus {
    bool p_err;
    std::string err_msg;
    void error(const std::string &err_msg);
    ParserStatus();
    ParserStatus(bool p_err, std::string err_msg);
};

struct NOP { void operator()(); };

// load a constant to the register
struct LoadConstantOp {
    LoadConstantOp(AntInteractor& interactor, long register_idx, cpu_word_size const value);
    void operator()();

private:
    AntInteractor& interactor;
    long register_idx;
    cpu_word_size const value;
};

struct MoveOp {
    MoveOp(AntInteractor& interactor, long dx, long dy);
    void operator()();
    AntInteractor& interactor;
    long dx, dy;
};

struct CopyOp {
    CopyOp(AntInteractor& interactor, long reg_src_idx, long reg_dst_idx);
    void operator()();
    AntInteractor& interactor;
    long reg_src_idx, reg_dst_idx;
};

struct AddOp {
    AddOp(AntInteractor& interactor, long reg_src_idx, long reg_dst_idx);
    void operator()();
    AntInteractor& interactor;
    long reg_src_idx, reg_dst_idx;
};

struct SubOp {
    SubOp(AntInteractor& interactor, long reg_src_idx, long reg_dst_idx);
    void operator()();
    AntInteractor& interactor;
    long reg_src_idx, reg_dst_idx;
};

struct IncOp {
    IncOp(AntInteractor& interactor, long register_idx);
    void operator()();
    AntInteractor& interactor;
    long register_idx;
};

struct DecOp {
    DecOp(AntInteractor& interactor, long register_idx);
    void operator()();
    AntInteractor& interactor;
    long register_idx;
};

struct JmpOp {
    union Address {
        Address(std::string* str_lbl): str_lbl(str_lbl) {}
        Address(size_t op_idx): op_idx(op_idx) {}
        std::string* str_lbl;
        size_t op_idx;
    } addr;
    enum Type { LABEL, INDEX } type;
    Operations& operations;

    JmpOp(std::string label, Operations& operations);
    JmpOp(size_t op_idx, Operations& operations);
    void operator()();
};

struct JnzOp: public JmpOp {
    JnzOp(AntInteractor& interactor, std::string label, Operations& operations);
    JnzOp(AntInteractor& interactor, size_t op_idx, Operations& operations);
    void operator()();
    AntInteractor& interactor;
};
