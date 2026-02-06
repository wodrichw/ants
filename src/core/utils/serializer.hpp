#pragma once

#include <fstream>
#include <libtcod/color.hpp>
#include <type_traits>
#include <vector>

#include "app/globals.hpp"
#include "utils/types.hpp"
#include "google/protobuf/io/coded_stream.h"
#include "google/protobuf/io/zero_copy_stream_impl_lite.h"
#include "google/protobuf/message_lite.h"
#include "spdlog/spdlog.h"

class Packer {
   public:
    Packer(std::string const& path);
    ~Packer();

    explicit operator bool();

    template <typename T>
    Packer& operator<<(T const& data) {
        std::string msg;
        if constexpr (std::is_base_of_v<google::protobuf::MessageLite, T>) {
            msg.reserve(static_cast<size_t>(data.ByteSizeLong()));
            google::protobuf::io::StringOutputStream string_output(&msg);
            google::protobuf::io::CodedOutputStream coded_output(&string_output);
            coded_output.SetSerializationDeterministic(true);

            if(!data.SerializeToCodedStream(&coded_output) ||
               coded_output.HadError()) {
                SPDLOG_ERROR("Failed to deterministically serialize object");
                return *this;
            }
        } else if(!data.SerializeToString(&msg)) {
            SPDLOG_ERROR("Failed to serialize object");
            return *this;
        }

        int size = static_cast<int>(msg.size());
        write_int(size);
        output.write(msg.data(), size);
        output.flush();

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

    bool is_valid();

   private:
    void close();
    int read_int();

    std::ifstream input;
    std::vector<char> buffer;
};
