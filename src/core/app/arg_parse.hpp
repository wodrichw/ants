#pragma once

#include <map>
#include <string>

class ArgumentParser {
   private:
    std::map<std::string, std::string> arguments = {};

   public:
    ArgumentParser();
    ArgumentParser(int argc, char* argv[]);


    bool hasKey(const std::string& key) const;

    std::string const& getString(const std::string& key,
                                 const std::string& default_value = "") const;
    int getInt(const std::string& key, int default_value = 0) const;
    double getDouble(const std::string& key, double default_value = 0.0) const;
    bool getBool(const std::string& key, bool default_value = false) const;
};

class ProjectArguments {
   private:
    ArgumentParser parser = {};
    void help() const;
    void setup_logging() const;

   public:
    std::string const default_map_file_path = {};
    std::string const save_path = {};
    bool const is_render = {};
    bool const is_debug_graphics = {};
    bool const is_walls_enabled = {};
    bool const no_fov = {};
    ProjectArguments(int argc, char* argv[]);
    ProjectArguments(
        std::string const& default_map_file_path,
        std::string const& save_path,
        bool is_render, bool is_debug_graphics, bool is_walls_enabled);
};
