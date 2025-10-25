#pragma once

#include <iostream>
#include <string>
#include <map>

class HttpRequest {
    
    private:

        std::string _buffer;
        std::string _method;
        std::string _path;
        std::string _httpVersion;
        std::map<std::string, std::string> _headers;
        std::string _body;
        std::string _queryString;

    public:

        HttpRequest(const char *buffer);
        ~HttpRequest();

        const std::string& getMethod() const;
        const std::string& getPath() const;
        const std::string& getHttpVersion() const;
        const std::string& getHeader(const std::string &key) const;
        const std::string& getBody() const;
        const std::string& getQueryString() const;
        int getContentLength() const;
};