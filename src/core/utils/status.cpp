#include "utils/status.hpp"

#include "spdlog/spdlog.h"

Status::Status() : p_err(false), err_msg("") {
    SPDLOG_DEBUG("Status created");
}
Status::Status(bool p_err, std::string err_msg)
    : p_err(p_err), err_msg(err_msg) {
        SPDLOG_DEBUG("Status created with error message");
    }
void Status::error(const std::string& err_msg) {
    SPDLOG_ERROR("Status error: {}", err_msg);
    this->p_err = true;
    this->err_msg = err_msg;
}
