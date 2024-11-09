#pragma once

#include <fstream>
#include <iostream>
#include <libtcod/color.hpp>
#include <vector>

#include "proto/utils.pb.h"
#include "spdlog/spdlog.h"

using ulong = unsigned long;

class Packer {
   public:
    Packer(std::string const& path) : output(path, std::ios::binary) {
        if(path == "") return;
        if(!output) {
            SPDLOG_ERROR("Failed to open file for writing: '{}'", path);
            return;
        }
    }
    ~Packer() { close(); }

    explicit operator bool() { return bool(output); }

    template <typename T>
    Packer& operator<<(T const& data) {
        std::string msg;
        if(!data.SerializeToString(&msg)) {
            SPDLOG_ERROR("Failed to serialize object");
            return *this;
        }

        int size = msg.size();
        write_int(size);
        output.write(msg.data(), size);

        return *this;
    }

    Packer& operator<<(tcod::ColorRGB const& col) {
        ant_proto::Integer msg;
        msg.set_value((col.r << 16) | (col.g << 8) | col.b);
        (*this) << msg;
        return *this;
    }

   private:
    void write_int(int value) {
        output.write(reinterpret_cast<const char*>(&value), sizeof(value));
    }

    void close() { output.close(); }

    std::ofstream output;
};

class Unpacker {
   public:
    Unpacker(std::string const& path) : input(path, std::ios::binary) {
        if (path == "") return;
        if(!input) {
            SPDLOG_ERROR("Failed to open file for reading: '{}'", path);
            return;
        }
    }
    ~Unpacker() { close(); }

    // Deserialize method
    template <typename T>
    Unpacker& operator>>(T& obj) {
        int size = read_int();

        buffer.resize(size);
        input.read(buffer.data(), size);

        std::string data(buffer.begin(), buffer.end());
        if(!obj.ParseFromString(data)) {
            SPDLOG_ERROR("Failed to parse serialized data.");
            exit(1);
        }
        return *this;
    }

    Unpacker& operator>>(tcod::ColorRGB& col) {
        ant_proto::Integer msg;
        (*this) >> msg;

        uint color = msg.value();
        col.r = color >> 16;
        col.g = (color >> 8) & 255;
        col.b = color & 255;
        return *this;
    }

    bool is_valid() const { return input ? true : false; }

   private:
    void close() { input.close(); }

    int read_int() {
        int value;
        input.read(reinterpret_cast<char*>(&value), sizeof(value));
        return value;
    }

    std::ifstream input;
    std::vector<char> buffer;
};

