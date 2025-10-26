#include "../include/HttpRequest.hpp"

#include <sstream>
#include <vector>
#include "../include/Utils.hpp"
#include <exception>

HttpRequest::HttpRequest(const char *buffer): _buffer(buffer) {
    std::string req(buffer);
    
    // encontrar o fim dos headers
    size_t headerEnd = req.find("\r\n\r\n");
    std::string headersPart = req.substr(0, headerEnd);
    _body = (headerEnd != std::string::npos) ? req.substr(headerEnd + 4) : "";

    std::vector<std::string> lines;
    std::stringstream ss(headersPart);
    std::string aux;

    while (getline(ss, aux, '\n')) {
        lines.push_back(Utils::trim(aux));
    }

    // primeira linha → método, caminho e versão
    std::stringstream firstLine(lines[0]);
    firstLine >> _method >> _path >> _httpVersion;

    // headers
    for (size_t i = 1; i < lines.size(); i++) {
        size_t pos = lines[i].find(':');
        if (pos == std::string::npos) continue;

        std::string key = Utils::trim(lines[i].substr(0, pos));
        std::string value = Utils::trim(lines[i].substr(pos + 1));
        _headers[key] = value;
    }
}


HttpRequest::~HttpRequest() {};

const std::string& HttpRequest::getHttpVersion() const {
    return this->_httpVersion;
};

const std::string& HttpRequest::getMethod() const {
    return this->_method;
};

const std::string& HttpRequest::getPath() const {
    return this->_path;
};

const std::string& HttpRequest::getHeader(const std::string &key) const {

    const std::map<std::string, std::string>::const_iterator it = this->_headers.find(key);

    if (it != this->_headers.end()) {
        return it->second;
    }
    return NULL;
};

const std::string& HttpRequest::getBody() const {
    return this->_body;
};

const std::string& HttpRequest::getQueryString() const {
    return this->_queryString;
};

int HttpRequest::getContentLength() const {
    std::map<std::string, std::string>::const_iterator it = _headers.find("Content-Length");
    if (it == _headers.end())
        return 0;
    std::string value = it->second;
    return std::atoi(value.c_str());
}
