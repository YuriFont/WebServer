#include "../include/Config.hpp"
#include "../include/Utils.hpp"

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
    do {
        iss.clear();
        iss.str(line);
        iss >> line;
        std::cout << "Parsing line: " << line << std::endl;
        if (line == "listen") {
            _parseListen(iss);
            std::cout << "Parsed listen directive: IP = " << this->ip << ", Port = " << this->port << std::endl;
        }
        else if (line == "server_name") {
            _parseServerName(iss);
            std::cout << "Parsed server_name directive: server_name = " << this->server_name << std::endl;
        }
        else if (line == "error_page") {
            _parseErrorPages(iss);
            std::cout << "Parsed error_page directive: error_code = " << (--this->error_page.end())->first << ", path = " << (--this->error_page.end())->second << std::endl;
        }
        else if (line == "client_max_body_size") {
            _parseClientMaxBodySize(iss);
            std::cout << "Parsed client_max_body_size directive: client_max_body_size = " << this->client_max_body_size << std::endl;
        }
        else if (line == "location") {
            _parseLocation(iss);
        }
        else if (line == "}")
            break;
        else
            throw std::runtime_error("Unknown directive: " + line + " in " + _filePath);
    } while (_getUtilLine(_file, line));
    _file.close();
}

void Config::_skipComments(std::string &line) {
    std::istringstream iss(line);
	std::getline(iss, line, ';');
	iss.clear();
	iss.str(line);
	std::getline(iss, line, '#');
    line = Utils::trim(line);
}

std::istream& Config::_getUtilLine(std::istream &is, std::string &line) {
    while (std::getline(is, line)) {
        _skipComments(line);
        if (!line.empty())
            return is;
    }
    return is;
}

void Config::_parseListen(std::istringstream &iss) {
    std::string addressIP;
    int addressPort;

    if (iss.str().find(':') == std::string::npos)
        throw std::runtime_error("Listen directive must be in the format IP:PORT in " + _filePath);
    std::getline(iss, addressIP, ':');
    std::string portStr = iss.str().substr(iss.str().find(':') + 1);
    addressPort = std::atoi(portStr.c_str());
    addressIP = Utils::trim(addressIP);
    if (!_IPValidation(addressIP))
        throw std::runtime_error("Invalid IP address: " + addressIP + " in " + _filePath);
    if (!_PortValidation(addressPort))
        throw std::runtime_error("Invalid port number: " + Utils::toString(addressPort) + " in " + _filePath);
    this->ip = (addressIP == "localhost") ? "127.0.0.1" : addressIP;
    this->port = addressPort;
}

bool Config::_IPValidation(const std::string &addressIP) {
    std::istringstream iss(addressIP);
    std::string segment;
    int count = 0;

    if (addressIP.empty() || addressIP[addressIP.size() - 1] == '.')
        return false;
    if (addressIP == "localhost")
        return true;
    while (std::getline(iss, segment, '.')) {
        count++;

        if (segment.empty())
            return false;

        for (size_t i = 0; i < segment.size(); i++) {
            if (!std::isdigit(segment[i]))
                return false;
        }

        if (segment.size() > 1 && segment[0] == '0')
            return false;

        int value = std::atoi(segment.c_str());
        if (value < 0 || value > 255)
            return false;
    }
    return (count == 4);
}

bool Config::_PortValidation(int addressPort) {
    if (addressPort < 1024 || addressPort > 65535) {
        return false;
    }
    return true;
}

void Config::_parseServerName(std::istringstream &iss) {
    std::string name;
    if (!(iss >> name) || !(iss.eof()))
        throw std::runtime_error("Invalid server_name directive in " + _filePath);
    this->server_name = name;
}

void Config::_parseErrorPages(std::istringstream &iss) {
    int errorCode;
    std::string errorPath;
    if (!(iss >> errorCode) || !(iss >> errorPath) || !(iss.eof()))
        throw std::runtime_error("Invalid error_page directive in " + _filePath);
    this->error_page[errorCode] = errorPath;
}

void Config::_parseClientMaxBodySize(std::istringstream &iss) {
    std::string sizeStr;
    if (!(iss >> sizeStr) || !(iss.eof()))
        throw std::runtime_error("Invalid client_max_body_size directive in " + _filePath);
    this->client_max_body_size = Utils::toSizeT(sizeStr);
}

void Config::_parseLocation(std::istringstream &iss) {
    
}