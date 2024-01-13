#pragma once 

#include <functional>
#include <unordered_map>
#include <vector>

class Operations {
    std::vector<std::function<void()>> _ops; 
    std::unordered_map<std::string, size_t> label_map;
public:
    size_t op_idx;
    bool goto_set;
    Operations(): _ops(), label_map(), op_idx(), goto_set() {}

    void add_op(const std::function<void()>&& op) { _ops.push_back(op); }
    void add_label(std::pair<std::string, size_t>&& p) { label_map.insert(p); }

    void set_op_idx(const std::string& label) { op_idx = label_map[label]; }
    void set_op_idx(size_t idx) { op_idx = idx; }
    const std::function<void()>& operator[](size_t idx) { return _ops[idx]; }
    size_t size() { return _ops.size(); }
};

struct ParserStatus {
    bool p_err;
    std::string err_msg;
    void error(const std::string& err_msg) {
        this->p_err = true;
        this->err_msg = err_msg;
    }
    ParserStatus(): p_err(false), err_msg("") {}
    ParserStatus(bool p_err, std::string err_msg): p_err(p_err), err_msg(err_msg) {}
};

struct EngineInteractor {
    using move_ant_f = std::function<void(int dx, int dy)>;

    move_ant_f move_ant;
    ParserStatus status;
    EngineInteractor() = default;
    EngineInteractor(const EngineInteractor& rhs);
};

struct MoveOp {
    EngineInteractor interactor;
    int dx, dy;
    MoveOp(const EngineInteractor& interactor, int dx, int dy);
    void operator()();
};

struct GotoOp {
    union Address {
        Address(std::string* str_lbl): str_lbl(str_lbl) {}
        Address(size_t op_idx): op_idx(op_idx) {}
        std::string* str_lbl;
        size_t op_idx;
    } addr;
    enum Type { LABEL, INDEX } type;
    Operations& operations;

    GotoOp(std::string label, Operations& operations);
    GotoOp(size_t op_idx, Operations& operations);
    void operator()();
};


