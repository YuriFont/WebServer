#include "../include/Location.hpp"

Location::Location() : _autoindex(false), _upload_enable(false) {}

Location::~Location() {}

std::string Location::getPath() const {
    return _path;
}

const std::vector<std::string>& Location::getMethods() const {
    return _methods;
}

std::string Location::getRoot() const {
    return _root;
}

const std::vector<std::string>& Location::getIndex() const {
    return _index;
}

bool Location::isAutoindex() const {
    return _autoindex;
}

std::string Location::getRedirect() const {
    return _redirect;
}

bool Location::isUploadEnabled() const {
    return _upload_enable;
}

std::string Location::getUploadStore() const {
    return _uploadStore;
}

const std::map<std::string, std::string>& Location::getCgi() const {
    return _cgi;
}

void Location::setPath(const std::string &p) {
    if (p.empty() || p[0] != '/')
        throw std::runtime_error("Location path must start with '/'");
    _path = p;
}

void Location::setMethods(std::istringstream &iss) {
    std::string method;
    while (iss >> method) {
        if (method != "GET" && method != "POST" && method != "DELETE" && method != "NONE")
            throw std::runtime_error("Invalid HTTP method: " + method);
        
        if (method == "NONE") {
            _methods.clear();
            break;
        }

        _methods.push_back(method);
    }
}

void Location::setRoot(std::istringstream &iss) {
    std::string rootPath;
    struct stat info;
    
    iss >> rootPath;
    if (iss.fail() || !iss.eof() || rootPath.empty())
        throw(std::runtime_error("Error in `root`: invalid format"));

    if (stat(rootPath.c_str(), &info) != 0)
        throw(std::runtime_error("Error in `root`: " + std::string(strerror(errno))));

    if (!S_ISDIR(info.st_mode))
        throw(std::runtime_error("Error in `root`: path is not a directory"));
    
    _root = rootPath;
}

void Location::setIndex(std::istringstream &iss) {
    std::string indexFile;
    while (iss >> indexFile) {
        _index.push_back(indexFile);
    }
    if (!iss.eof())
        throw std::runtime_error("Invalid format for index directive");
}

void Location::setAutoindex(std::istringstream &iss) {
    (void)iss;
    _autoindex = true;
}

void Location::setRedirect(std::istringstream &iss) {
    _redirect = iss.str();
}

void Location::setUploadEnabled(bool enable) {
    _upload_enable = enable;
}

void Location::setUploadStore(std::istringstream &iss) {
    setUploadEnabled(true);
    _uploadStore = iss.str();
}

void Location::addCgi(std::istringstream &iss) {
    (void)iss;
    _cgi["test"] = "test";
}
