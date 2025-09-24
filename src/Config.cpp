#include "../include/Config.hpp"

Config::Config(const std::string &filePath) : filePath(filePath) {
    parseConfigFile();
}

Config::~Config() {}

void Config::parseConfigFile() {
    std::ifstream file(filePath.c_str());
    if (!file.is_open()) {
        throw std::runtime_error("Could not open config file: " + filePath);
    }
    std::string line;
    while (std::getline(file, line)) {
        // Here you would parse each line of the configuration file
        std::cout << "Config Line: " << line << std::endl; // Placeholder
    }
    file.close();
}