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

std::string Location::getIndex() const {
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
    _path = p;
}

void Location::setMethods(std::istringstream &iss) {
    _methods.push_back(iss.str());
}

void Location::setRoot(std::istringstream &iss) {
    _root = iss.str();
}

void Location::setIndex(std::istringstream &iss) {
    _index = iss.str();
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
