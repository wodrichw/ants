#include <iostream>

#include "utils/serializer.hpp"
#include "proto/utils.pb.h"
#include "spdlog/spdlog.h"

Packer::Packer(std::string const& path) : output(path, std::ios::binary) {
    if(path == "") return;
    if(!output) {
        SPDLOG_ERROR("Failed to open file for writing: '{}'", path);
        return;
    }
}

Packer::~Packer() {
   close();
}

Packer::operator bool() {
    return bool(output);
}


Packer& Packer::operator<<(tcod::ColorRGB const& col) {
    ant_proto::Integer msg;
    msg.set_value((col.r << 16) | (col.g << 8) | col.b);
    (*this) << msg;
    return *this;
}

void Packer::write_int(int value) {
    output.write(reinterpret_cast<const char*>(&value), sizeof(value));
}

void Packer::close() {
    output.close();
}

Unpacker::Unpacker(std::string const& path) : input(path, std::ios::binary) {
    if (path == "") return;
    if(!input) {
        SPDLOG_ERROR("Failed to open file for reading: '{}'", path);
        return;
    }
}

Unpacker::~Unpacker() {
    close();
}

Unpacker& Unpacker::operator>>(tcod::ColorRGB& col) {
    ant_proto::Integer msg;
    (*this) >> msg;

    uint color = msg.value();
    col.r = color >> 16;
    col.g = (color >> 8) & 255;
    col.b = color & 255;
    return *this;
}

bool Unpacker::is_valid() const {
    return input ? true : false;
}

void Unpacker::close() {
    input.close();
}

int Unpacker::read_int() {
    int value;
    input.read(reinterpret_cast<char*>(&value), sizeof(value));
    return value;
}

