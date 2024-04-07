#include "arg_parse.hpp"
#include "spdlog/spdlog.h"

#include <iostream>
// ArgumentParser
// ============================================================================
ArgumentParser::ArgumentParser(int argc, char* argv[]) {
    for(int i = 1; i < argc; i += 2) {
        // Check if the argument is a key
        if (argv[i][0] != '-' || argv[i][1] != '-'){
            std::cerr << "Invalid argument key: " << argv[i] << std::endl;
            exit(1);
        }

        // Check if the argument does not have a value
        // It doesn't have a value if it is the last argument or the next argument is a key
        if(i + 1 >= argc || argv[i + 1][0] == '-' || argv[i + 1][1] == '-') {
            arguments[argv[i] + 2] = "1";
            --i; // decrement i so that the next iteration will not skip an argument
            continue;
        }

        // Add the key and value to the arguments map
        arguments[argv[i] + 2] = argv[i + 1];
    }
}

bool ArgumentParser::hasKey(const std::string& key) const {
    return arguments.find(key) != arguments.end();
}

std::string const& ArgumentParser::getString(const std::string& key, const std::string& default_value) const {
    if(hasKey(key)) {
        return arguments.at(key);
    }
    return default_value;
}

int ArgumentParser::getInt(const std::string& key, int default_value) const {
    if(hasKey(key)) {
        return std::stoi(arguments.at(key));
    }
    return default_value;
}

double ArgumentParser::getDouble(const std::string& key, double default_value) const {
    if(hasKey(key)) {
        return std::stod(arguments.at(key));
    }
    return default_value;
}

bool ArgumentParser::getBool(const std::string& key, bool default_value) const {
    if(hasKey(key)) {
        return arguments.at(key) == "true" || arguments.at(key) == "1";
    }
    return default_value;
}

// ProjectArguments
// ============================================================================

ProjectArguments::ProjectArguments(int argc, char* argv[]): parser(argc, argv),
    default_map_file_path(parser.getString("map_path")),
    is_render(!parser.getBool("no_render", false)) {

    if (parser.hasKey("help")) {
        help();
        exit(0);
    }
    setup_logging();
}

void ProjectArguments::help() const {
    std::cout << "Usage: ants [options]\n";
    std::cout << "Options:\n";
    std::cout << "  --map_path <path>  Path to the map file\n";
    std::cout << "  --log_level <level>  Set the runtime log level - options: trace, debug, info, warn, error, critical, and off. default: info. Note that this does not override the compile time log level.\n";
}

void ProjectArguments::setup_logging() const {
    std::string log_level = parser.getString("log_level", "info");
    for (auto & c: log_level) c = std::toupper((unsigned char)c);

    // change log pattern
    spdlog::set_pattern("%^[%l]: %v%$");

    if (log_level == "TRACE") {
        spdlog::set_level(spdlog::level::trace);
    } else if (log_level == "DEBUG") {
        spdlog::set_level(spdlog::level::debug);
    } else if (log_level == "INFO") {
        spdlog::set_level(spdlog::level::info);
    } else if (log_level == "WARN") {
        spdlog::set_level(spdlog::level::warn);
    } else if (log_level == "ERROR") {
        spdlog::set_level(spdlog::level::err);
    } else if (log_level == "CRITICAL") {
        spdlog::set_level(spdlog::level::critical);
    } else if (log_level == "OFF") {
        spdlog::set_level(spdlog::level::off);
    } else {
        std::cerr << "Invalid log level: " << log_level << std::endl;
        exit(1);
    }

    SPDLOG_TRACE("Trace logging enabled");
    SPDLOG_DEBUG("Debug logging enabled");
    SPDLOG_INFO("Info logging enabled");
    SPDLOG_WARN("Warn logging enabled");
    SPDLOG_ERROR("Error logging enabled");
    SPDLOG_CRITICAL("Critical logging enabled");
}