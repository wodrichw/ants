#include "utils/status.hpp"

#include "spdlog/spdlog.h"

Status::Status() : p_err(false), err_msg("") { SPDLOG_DEBUG("Status created"); }
Status::Status(bool p_err, std::string error_message)
    : p_err(p_err), err_msg(error_message) {
    SPDLOG_DEBUG("Status created with error message");
}
void Status::error(const std::string& error_message) {
    SPDLOG_ERROR("Status error: {}", error_message);
    this->p_err = true;
    this->err_msg = error_message;
}
