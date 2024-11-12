#pragma once

#include <fstream>
#include <libtcod/color.hpp>
#include <vector>

#include "spdlog/spdlog.h"

using ulong = unsigned long;

class Packer {
   public:
    Packer(std::string const& path);
    ~Packer();

    explicit operator bool();

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
    Packer& operator<<(tcod::ColorRGB const& col);

   private:
    void write_int(int value);
    void close();

    std::ofstream output;
};

class Unpacker {
   public:
    Unpacker(std::string const& path);
    ~Unpacker();

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

    Unpacker& operator>>(tcod::ColorRGB& col);

    bool is_valid() const;

   private:
    void close();
    int read_int();

    std::ifstream input;
    std::vector<char> buffer;
};

