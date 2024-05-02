#include "hardware/operations.hpp"

#include <string>

#include "app/globals.hpp"
#include "entity/map_entity.hpp"
#include "entity/map.hpp"
#include "spdlog/spdlog.h"

Operations::Operations() : _ops(), label_map(), op_idx() { SPDLOG_DEBUG("Operations created"); }
void Operations::add_op(std::function<bool()> const& op) {
    _ops.push_back(op);
}
void Operations::add_label(std::string const& label, size_t idx) {
    SPDLOG_DEBUG("Adding label {} at index {}", label, idx);
    auto it = label_map.find(label);
    if (it != label_map.end()) {
        it->second = idx;
        return;
    }
    label_map[label] = idx;
}
size_t const& Operations::get_label_idx(std::string const& label) {
    SPDLOG_DEBUG("Getting label index for {}", label);
    auto it = label_map.find(label);
    if (it != label_map.end()) {
        return it->second;
    }
    add_label(label);
    return label_map.find(label)->second;
}
void Operations::set_op_idx(const std::string& label) {
    SPDLOG_DEBUG("Setting op_idx to {}", label);
    op_idx = label_map[label];
}
void Operations::set_op_idx(size_t idx) {
    SPDLOG_DEBUG("Setting op_idx to {}", idx);
    op_idx = idx;
}
const std::function<bool()>& Operations::operator[](size_t idx) {
    SPDLOG_TRACE("Getting operation at index {}", idx);
    return _ops[idx];
}
size_t Operations::size() { return _ops.size(); }

void Operations::handleClockPulse() {
    SPDLOG_TRACE("Handling clock pulse for operations");
    bool op_result = true;
    for (int i = 0; i < 500 && op_idx < size() && op_result; ++i) {
        op_result = (*this)[op_idx]();
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
