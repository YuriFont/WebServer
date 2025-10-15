#include "../include/Location.hpp"
#include "../include/Utils.hpp"

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

    std::cout << rootPath << std::endl;
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
    std::string value;

    iss >> value;
    if (iss.fail() || !iss.eof())
        throw std::runtime_error("Invalid format for autoindex directive");

    if (value == "off")
        _autoindex = false;
    else if (value == "on")
        _autoindex = true;
    else
        throw std::runtime_error("Autoindex must be 'on' or 'off'");
}

void Location::setRedirect(std::istringstream &iss) {
    std::string statusCode;
    std::string url;
    int wordCount = Utils::countWords(iss);
    
    if (wordCount < 2 || wordCount > 3)
        throw std::runtime_error("Invalid format for redirect directive");

    if (wordCount == 3) {
        iss >> statusCode;
        if (statusCode != "301" && statusCode != "302")
            throw std::runtime_error("Invalid status code for redirect: " + statusCode);
        _redirectCode = std::atoi(statusCode.c_str());
    }
    else
        _redirectCode = 302;

    iss >> url;

    if (!Utils::isValidUrl(url))
        throw std::runtime_error("Invalid URL for redirect: " + url);

    _redirect = url;
}

void Location::setUploadEnabled(bool enable) {
    _upload_enable = enable;
}

void Location::setUploadStore(std::istringstream &iss) {
    std::string rootUploadPath;
    struct stat info;

    setUploadEnabled(true);
    iss >> rootUploadPath;
    if (iss.fail() || !iss.eof() || rootUploadPath.empty())
        throw(std::runtime_error("Error in `upload_store`: invalid format"));

    if (stat(rootUploadPath.c_str(), &info) != 0)
        throw(std::runtime_error("Error in `upload_store`: " + std::string(strerror(errno))));

    if (!S_ISDIR(info.st_mode))
        throw(std::runtime_error("Error in `upload_store`: path is not a directory"));

    _uploadStore = rootUploadPath;
}

void Location::addCgi(std::istringstream &iss) {
    std::string extension;
    std::string scriptPath;
    struct stat info;

    if (this->getRoot().empty())
        throw(std::runtime_error("Error in `cgi`: root must be set before cgi"));
    iss >> extension >> scriptPath;
    if (iss.fail() || !iss.eof() || extension.empty() || scriptPath.empty())
        throw(std::runtime_error("Error in `cgi`: invalid format"));

    if (stat(scriptPath.c_str(), &info) != 0)
        throw(std::runtime_error("Error in `cgi`: " + std::string(strerror(errno))));

    if (!S_ISREG(info.st_mode) || !(info.st_mode & S_IXUSR))
        throw(std::runtime_error("Error in `cgi`: path is not an executable file"));

    _cgi[extension] = scriptPath;
}
