#include "../include/HttpRequest.hpp"

#include <sstream>
#include <vector>
#include "../include/Utils.hpp"
#include <exception>

HttpRequest::HttpRequest(char *buffer): _buffer(buffer) {

    // requisição
    
    std::vector<std::string> lines;
    std::string aux;
    //    GET / HTTP/1.1
    //    Host: localhost:4242
    //    Connection: keep-alive
    //    sec-ch-ua: "Google Chrome";v="141", "Not?A_Brand";v="8", "Chromium";v="141"
    //    sec-ch-ua-mobile: ?0
    //    sec-ch-ua-platform: "Windows"
    //    Upgrade-Insecure-Requests: 1
    //    User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/141.0.0.0 Safari/537.36
    //    Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/avif,image/webp,image/apng,*/*;q=0.8,application/signed-exchange;v=b3;q=0.7
    //    Sec-Fetch-Site: none
    //    Sec-Fetch-Mode: navigate
    //    Sec-Fetch-User: ?1
    //    Sec-Fetch-Dest: document
    //    Accept-Encoding: gzip, deflate, br, zstd
    //    Accept-Language: pt-BR,pt;q=0.9,en-US;q=0.8,en;q=0.7,ja;q=0.6
    std::stringstream ss(this->_buffer);

    while (getline(ss, aux, '\n')) {
        lines.push_back(aux);
    }
    ss.clear();
    ss.str(lines[0]);

    int option = 0;
    while (getline(ss, aux, ' ')) {
        
        if (option == 0) {
            this->_method = Utils::trim(aux);
        } else if (option == 1) {
            this->_path = Utils::trim(aux);
        } else if (option == 2) {
            this->_httpVersion = Utils::trim(aux);
            break ;
        }
        option++;
    }

    for (size_t i = 1; i < lines.size(); i++) {

        size_t pos = lines[i].find(':');
        if (pos == std::string::npos) {
            continue ; 
        }
        std::string key = Utils::trim(lines[i].substr(0, pos));
        std::string value = Utils::trim(lines[i].substr(pos + 1, lines[i].size()));
        this->_headers[key] = value;
    }

    std::cout << this->getMethod() << " " << this->getPath() << " " << this->getHttpVersion() << std::endl;
};

HttpRequest::~HttpRequest() {

};

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