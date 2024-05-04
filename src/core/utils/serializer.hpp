#pragma once

#include <iostream>
#include <fstream>
#include <vector>

#include "spdlog/spdlog.h"

using ulong = unsigned long;

class Packer {

    public:

    template <typename T>
    Packer& operator<<(T const& obj) {
        const uint8_t* bytes = reinterpret_cast<const uint8_t*>(obj);
        buffer.insert(buffer.end(), bytes, bytes + sizeof(obj));
        return *this;
    }

    void write(const std::string& filename) {
        std::ofstream file(filename, std::ios::binary);
        if (!file.is_open()) {
            SPDLOG_ERROR("Failed to write to file: '{}'", filename);
            return;
        }

        file.write(reinterpret_cast<const char*>(buffer.data()), buffer.size());
        file.close();
        SPDLOG_INFO("{} byte(s) was written to '{}' successfully.", buffer.size(), filename);
    }

    private:
    std::vector<uint8_t> buffer;
};

class Unpacker {

public:

    Unpacker(std::string const& filename) {
        read(filename);
    }

    // Deserialize method
    template <typename T>
    Unpacker& operator>>(T& obj) {
        std::memcpy(&obj, buffer.data() + offset, sizeof(obj));
        offset += sizeof(obj);
        return *this;
    }

    private:

    void read(const std::string& filename) {
        std::ifstream file(filename, std::ios::binary | std::ios::ate);
        if (!file.is_open()) {
            SPDLOG_ERROR("Failed to read binary file: '{}'", filename);
            return;
        }

        std::streamsize size = file.tellg();
        file.seekg(0, std::ios::beg);

        buffer.reserve(size);
        offset = 0;
        if (!file.read(reinterpret_cast<char*>(buffer.data()), size)) {
            SPDLOG_ERROR("Unable to read data from binary file: '{}'", filename);
            return;
        }
        
        SPDLOG_INFO("Read {} byte(s) from the file successfully.");
    }

    std::vector<uint8_t> buffer;
    size_t offset = 0;
};
