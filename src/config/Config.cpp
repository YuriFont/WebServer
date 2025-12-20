#include "../../include/config/Config.hpp"
#include "../../include/config/Location.hpp"
#include "../../include/core/ServerConfig.hpp"
#include "../../include/utils/Utils.hpp"

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
    _file.close();
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
    ServerConfig server;
    if (!_getUtilLine(_file, line))
        throw std::runtime_error("This server block is invalid: " + _filePath);

    std::istringstream iss(line);
    do {
        iss.clear();
        iss.str(line);
        iss >> line;
        if (line == "listen")
            _parseListen(iss, server);
        else if (line == "server_name")
            _parseServerName(iss, server);
        else if (line == "error_page")
            _parseErrorPages(iss, server);
        else if (line == "client_max_body_size")
            _parseClientMaxBodySize(iss, server);
        else if (line == "location")
            _parseLocation(iss, server);
        else if (line == "}")
            break;
        else
            throw std::runtime_error("Unknown directive: " + line + " in " + _filePath);
    } while (_getUtilLine(_file, line));
    this->servers.push_back(server);
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

void Config::_parseListen(std::istringstream &iss, ServerConfig &server) {
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
    if (!_repeatedPortValidation(addressPort))
        throw std::runtime_error("This door is already in use: " + Utils::toString(addressPort) + " in " + _filePath);
    if (_isPortInUseOnLocalhost(addressPort))
        throw std::runtime_error("This door is already in use: " + Utils::toString(addressPort) + " in " + _filePath);

    server.ip = (addressIP == "localhost") ? "127.0.0.1" : addressIP;
    server.port = addressPort;
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

bool Config::_repeatedPortValidation(const int addressPort) {
    for (std::vector<ServerConfig>::const_iterator it = servers.begin(); it != servers.end(); ++it) {
        if (it->getPort() == addressPort)
            return false;
    }
    return true;
}

bool Config::_isPortInUseOnLocalhost(int port) {
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
        throw std::runtime_error("socket() failed");

    sockaddr_in addr;
    std::memset(&addr, 0, sizeof(addr));

    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);

    if (inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr) <= 0) {
        close(sockfd);
        throw std::runtime_error("inet_pton() failed");
    }

    bool inUse = false;

    if ((bind(sockfd, (struct sockaddr *)&addr, sizeof(addr)) < 0) && (errno == EADDRINUSE))
        inUse = true;

    close(sockfd);

    return inUse;
}

void Config::_parseServerName(std::istringstream &iss, ServerConfig &server) {
    std::string name;

    if (!(iss >> name) || !(iss.eof()))
        throw std::runtime_error("Invalid server_name directive in " + _filePath);
    server.server_name = name;
}

void Config::_parseErrorPages(std::istringstream &iss, ServerConfig &server) {
    int errorCode;
    std::string errorPath;

    if (!(iss >> errorCode) || !(iss >> errorPath) || !(iss.eof()))
        throw std::runtime_error("Invalid error_page directive in " + _filePath);
    server.error_pages[errorCode] = errorPath;
}

void Config::_parseClientMaxBodySize(std::istringstream &iss, ServerConfig &server) {
    std::string sizeStr;

    if (!(iss >> sizeStr) || !(iss.eof()))
        throw std::runtime_error("Invalid client_max_body_size directive in " + _filePath);
    server.client_max_body_size = Utils::toSizeT(sizeStr);
}

bool Config::checkGlobalCGI(const std::string &path) {
    if (path.size() < 5)
        throw (std::runtime_error("Error in `cgi Global`: Invalid global CGI path"));

    if (path[path.size() - 1] != '$')
        throw (std::runtime_error("Error in `cgi Global`: Global CGI location must end with '$'"));

    if (path[1] != '/' || path[2] != '.')
        throw (std::runtime_error("Error in `cgi Global`: Invalid global CGI path"));

    for (std::string::size_type i = 3; i < path.size() - 1; ++i) {
        char c = path[i];
        if (!((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9'))) {
            throw (std::runtime_error("Error in `cgi Global`: Invalid global CGI path"));
        }
    }

    return true;
}

void Config::addGlobalCGI(ServerConfig &server, std::istringstream &iss, std::string &line) {
    int flagOfCgi = 0;
    std::string ext = line.substr(2, line.size() - 3);

    if (server.hasExtGlobalCgi(ext))
        throw std::runtime_error("Error in global cgi block: global cgi extensions are equal");

    if (!(iss >> line) || line != "{")
        throw std::runtime_error("Expected '{' after location path in " + _filePath);

    while (_getUtilLine(_file, line)) {
        iss.clear();
        iss.str(line);
        iss >> line;
        if (line == "}")
            break;
        else if (line == "methods")
            server.setGlobalCgiMethods(iss, ext);
        else if (line == "cgi") {
            flagOfCgi++;
            server.setGlobalCgiPath(iss, ext);
        }
        else
            throw std::runtime_error("Unknown directive in global cgi block: " + line + " in " + _filePath);
    }

    if (server.extAndMethods.empty())
        throw std::runtime_error("Error in global cgi block: global CGI need methods");

    if (server.extAndPath.empty())
        throw std::runtime_error("Error in global cgi block: global CGI need cgi key");
    
    if (flagOfCgi != 1)
        throw std::runtime_error("Error in global cgi block: global CGI accept just one cgi key for location");

    server.hasGlobalCGI = true;
}

void Config::_parseLocation(std::istringstream &iss, ServerConfig &server) {
    Location location;
    std::string line;

    if (!(iss >> line))
        throw std::runtime_error("Invalid location directive in " + _filePath);
    
    if (line[0] == '~' && checkGlobalCGI(line)) {
        addGlobalCGI(server, iss, line);
        return ;
    }

    location.setPath(line);

    if (!(iss >> line) || line != "{")
        throw std::runtime_error("Expected '{' after location path in " + _filePath);

    while (_getUtilLine(_file, line)) {
        iss.clear();
        iss.str(line);
        iss >> line;
        if (line == "}")
            break;
        else if (line == "root")
            location.setRoot(iss);
        else if (line == "index")
            location.setIndex(iss);
        else if (line == "methods")
            location.setMethods(iss);
        else if (line == "redirect")
            location.setRedirect(iss);
        else if (line == "autoindex")
            location.setAutoindex(iss);
        else if (line == "upload_store")
            location.setUploadStore(iss);
        else if (line == "cgi")
            location.addCgi(iss);
        else
            throw std::runtime_error("Unknown directive in location block: " + line + " in " + _filePath);
    }

    if (location.getMethods().empty()) {
        std::istringstream iss("GET POST DELETE");
        location.setMethods(iss);
    }

    if (server.locations.find(location.getPath()) != server.locations.end())
        throw std::runtime_error("Duplicate location path: " + location.getPath() + " in " + _filePath);

    if (location.isMethodAllowed("GET") && location.getRoot().empty() && location.getRedirect().empty())
        throw std::runtime_error("No root and redirecion defined for location: " + location.getPath() + " in " + _filePath);

    if (location.isMethodAllowed("POST") && location.getUploadStore().empty() && location.getRoot().empty() && location.getRedirect().empty())
        throw std::runtime_error("POST method requires upload_store or root to be set for location: " + location.getPath() + " in " + _filePath);

    if (location.isMethodAllowed("POST") == false && location.isUploadEnabled())
        throw(std::runtime_error("POST method must be allowed for uploads in location: " + location.getPath() + " in " + _filePath));

    server.locations[location.getPath()] = location;
}
