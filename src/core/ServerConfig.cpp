#include "../../include/core/ServerConfig.hpp"

ServerConfig::ServerConfig(): hasGlobalCGI(false) {
    this->server_name = "";
    this->ip = "";
    this->port = 0;
    this->error_pages = std::map<int, std::string>();
    this->client_max_body_size = 0;
    this->locations = std::map<std::string, Location>();
}

ServerConfig::~ServerConfig() {
}

ServerConfig::ServerConfig(const ServerConfig& server) {
    *this = server;
}

ServerConfig& ServerConfig::operator=(const ServerConfig& other) {
    if (this != &other) {
        this->server_name = other.server_name;
        this->ip = other.ip;
        this->port = other.port;
        this->error_pages = other.error_pages;
        this->client_max_body_size = other.client_max_body_size;
        this->socket_fd = other.socket_fd;
        this->locations = other.locations;
        this->hasGlobalCGI = other.hasGlobalCGI;
        this->extAndMethods = other.extAndMethods;
        this->extAndPath = other.extAndPath;
    }
    return *this;
}

int ServerConfig::getSocket() const{
    return this->socket_fd;
}

const std::map<std::string, Location>& ServerConfig::getLocations() const{
    return this->locations;
}

std::string ServerConfig::getIp() const{
    return this->ip;
}

int ServerConfig::getPort() const{
    return this->port;
}

int ServerConfig::initSocket() {
    socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd == -1){
        std::cerr << "Error in socket: " << strerror(errno) << std::endl;
        return -1;
        // exit(EXIT_FAILURE);
    }
    fcntl(socket_fd, F_SETFL, O_NONBLOCK);
    int opt = 1;
    if (setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
    {
        std::cerr << "Error in setsockopt: " << strerror(errno) << std::endl;
        close(socket_fd);
        return -1;
        // exit(EXIT_FAILURE);
    }
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = inet_addr(ip.c_str());
    if (addr.sin_addr.s_addr == INADDR_NONE)
    {
        std::cerr << "Invalid IP: " << ip << std::endl;
        return -1;
        // exit(EXIT_FAILURE);
    }
    if (bind(socket_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0)
    {
        std::cerr << "Error in bind: " << strerror(errno) << std::endl;
        close(socket_fd);
        return -1;
        // exit(EXIT_FAILURE);
    }
    if (listen(socket_fd, SOMAXCONN) < 0)
    {
        std::cerr << "Error in listen: " << strerror(errno) << std::endl;
        close(socket_fd);
        return -1;
        // exit(EXIT_FAILURE);
    }
    std::cout << "Server listening on " << ip << ":" << port << std::endl;
    return socket_fd;
}

void ServerConfig::setGlobalCgiMethods(std::istringstream &iss, std::string &ext) {
    std::string method;

    while (iss >> method) {
        if (method != "GET" && method != "POST" && method != "DELETE")
            throw std::runtime_error("Invalid HTTP method in global cgi: " + method);

        extAndMethods[ext].push_back(method);
    }
}

void ServerConfig::setGlobalCgiPath(std::istringstream &iss, std::string &ext) {
    std::string extension;
    std::string scriptPath;
    struct stat info;

    iss >> extension >> scriptPath;
    if (iss.fail() || !iss.eof() || extension.empty() || scriptPath.empty())
        throw(std::runtime_error("Error in global `cgi`: invalid format"));

    if (stat(scriptPath.c_str(), &info) != 0)
        throw(std::runtime_error("Error in global `cgi`: " + std::string(strerror(errno))));

    if (!S_ISREG(info.st_mode) || !(info.st_mode & S_IXUSR))
        throw(std::runtime_error("Error in global `cgi`: path is not an executable file"));

    if (extension != ext)
        throw(std::runtime_error("Error in global `cgi`: extesion in cgi key is different of path extension"));

    extAndPath[ext] = scriptPath;
}

bool ServerConfig::hasExtGlobalCgi(std::string &ext) const {
    return (extAndPath.find(ext) != extAndPath.end());
}
