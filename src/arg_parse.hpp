#pragma once

#include <string>
#include <map>
#include <iostream>

class ArgumentParser {
private:
    std::map<std::string, std::string> arguments;

public:
    ArgumentParser(int argc, char* argv[]);

    bool hasKey(const std::string& key);

    std::string getString(const std::string& key, const std::string& default_value="");
    int getInt(const std::string& key, int default_value=0);
    double getDouble(const std::string& key, double default_value=0.0);
    bool getBool(const std::string& key, bool default_value=false);
};

class ProjectArguments {
private:
    ArgumentParser parser;
    void help() const;
public:
    const std::string default_map_file_path;
    ProjectArguments(int argc, char* argv[]);
};