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
    std::string line;

    if (!_getUtilLine(_file, line))
        throw std::runtime_error("This server block is invalid: " + _filePath);
    std::istringstream iss(line);
    while (_getUtilLine(_file, line)) {
        iss.clear();
        iss.str(line);
        std::cout << "Parsing line: " << line << std::endl;
        if (line == "listen")
            _parseListen(iss, line);
        //else if (line == "server_name")
            //_parseServerName(iss, line);
        //else if (line == "error_page")
            //_parseErrorPage(iss, line);
        //else if (line == "client_max_body_size")
            //_parseClientMaxBodySize(iss, line);
        //else if (line == "location")
            //_parseLocation(iss, line);
        else if (line == "}")
            return;
        else
            throw std::runtime_error("Unknown directive: " + line + " in " + _filePath);
    }
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

std::istream& Config::_getUtilLine(std::istream &is, std::string &line) {
    while (std::getline(is, line)) {
        _skipComments(line);
        if (!line.empty())
            return is;
    }
    return is;
}

void Config::_parseListen(std::istringstream &iss, std::string &line) {
    std::string addressIP;
    int addressPort;
    
}