#pragma once 

#include "globals.hpp"
#include <functional>
#include <unordered_map>
#include <vector>
#include <string>

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
};

struct ParserStatus {
    bool p_err;
    std::string err_msg;
    void error(const std::string &err_msg);
    ParserStatus();
    ParserStatus(bool p_err, std::string err_msg);
};

struct EngineInteractor {
    using move_ant_f = std::function<void(int dx, int dy)>;

    move_ant_f move_ant;
    ParserStatus status;
    EngineInteractor() = default;
    EngineInteractor(const EngineInteractor& rhs);
};

struct BrainInteractor {
    std::function<cpu_word_size&(int idx)> get_register;
    std::function<void(int idx, cpu_word_size value)> write_register;
};

struct NOP { void operator()(); };

// load a constant to the register
struct LoadConstantOp {
  LoadConstantOp(cpu_word_size &reg, cpu_word_size const value);
  void operator()();

private:
  cpu_word_size &reg;
  cpu_word_size const value;
};

struct MoveOp {
    EngineInteractor interactor;
    int dx, dy;
    MoveOp(const EngineInteractor& interactor, int dx, int dy);
    void operator()();
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
