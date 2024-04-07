#include "arg_parse.hpp"

// ArgumentParser
// ============================================================================
ArgumentParser::ArgumentParser(int argc, char* argv[]) {
    for(int i = 1; i < argc; i += 2) {
        // Check if the argument is a key
        if (argv[i][0] != '-' || argv[i][1] != '-') continue;

        // Check if the argument does not have a value
        // It doesn't have a value if it is the last argument or the next argument is a key
        if(i + 1 >= argc || argv[i + 1][0] != '-' || argv[i + 1][1] != '-') {
            arguments[argv[i] + 2] = "1";
            continue;
        }

        // Add the key and value to the arguments map
        arguments[argv[i] + 2] = argv[i + 1];
    }
}

bool ArgumentParser::hasKey(const std::string& key) {
    return arguments.find(key) != arguments.end();
}

std::string ArgumentParser::getString(const std::string& key, const std::string& default_value) {
    if(hasKey(key)) {
        return arguments[key];
    }
    return default_value;
}

int ArgumentParser::getInt(const std::string& key, int default_value) {
    if(hasKey(key)) {
        return std::stoi(arguments[key]);
    }
    return default_value;
}

double ArgumentParser::getDouble(const std::string& key, double default_value) {
    if(hasKey(key)) {
        return std::stod(arguments[key]);
    }
    return default_value;
}

bool ArgumentParser::getBool(const std::string& key, bool default_value) {
    if(hasKey(key)) {
        return arguments[key] == "true" || arguments[key] == "1";
    }
    return default_value;
}

// ProjectArguments
// ============================================================================

ProjectArguments::ProjectArguments(int argc, char* argv[]): parser(argc, argv),
    default_map_file_path(parser.getString("map_path")) {

    if (parser.hasKey("help")) {
        help();
        exit(0);
    }
}

void ProjectArguments::help() const {
    std::cout << "Usage: ants [options]\n";
    std::cout << "Options:\n";
    std::cout << "  --map_path <path>  Path to the map file\n";
}