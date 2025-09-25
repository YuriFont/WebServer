#include "../include/Config.hpp"

Config::Config(const std::string &filePath) : _filePath(filePath) {
    std::string line;

    _openFile();
    while (std::getline(_file, line)) {
        _skipComments(line);
        if (line.empty())
            continue;
        std::istringstream iss(line);
        if ((iss >> line) && (line == "server") && (iss >> line) && (line == "{") && (iss.eof()))
            _parseConfigFile();
        else
            throw std::runtime_error("Wrong initialization syntax: " + _filePath);   
    }
    std::cout << GREEN << "[Config]: Config file parsed successfully." << RESET << std::endl;
}

Config::~Config() {}

void Config::_openFile() {
    _file.open(_filePath.c_str());
    if (!_file.is_open())
        throw std::runtime_error("Could not open config file: " + _filePath);
    std::cout << GREEN << "[Config]: Config file opened successfully." << RESET << std::endl;
}

void Config::_parseConfigFile() {
    /*std::string line;
    while (std::getline(_file, line)) {

    }*/
    _file.close();
}

void Config::_skipComments(std::string &line) {
    std::istringstream iss(line);
	std::getline(iss, line, ';');
	iss.clear();
	iss.str(line);
	std::getline(iss, line, '#');
    size_t first = line.find_first_not_of(" \t");
    size_t last = line.find_last_not_of(" \t");
    if (first == std::string::npos || last == std::string::npos) {
        line.clear();
    } else {
        line = line.substr(first, last - first + 1);
    }
}