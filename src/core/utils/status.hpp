#pragma once

#include <string>

struct Status {
    bool p_err;
    std::string err_msg;

    Status();
    Status(bool p_err, std::string err_msg);
    void error(const std::string& err_msg);
};