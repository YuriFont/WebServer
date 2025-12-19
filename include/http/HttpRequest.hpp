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

        std::string bodyTempPath;
        HttpRequest();
        HttpRequest(const char *buffer);
        ~HttpRequest();

        const std::string& getMethod() const;
        const std::string& getPath() const;
        const std::string& getHttpVersion() const;
        const std::string& getHeader(const std::string &key) const;
        const std::string& getBody() const;
        const std::string& getQueryString() const;
        const std::string& getBuffer() const;
        int getContentLength() const;
        void appendBuffer(const std::string& buffer);
        void appendBody(const std::string& body);
        void clearAllData();
        void parser();
        void reserveSpaceBody(size_t size);
        void eraseBody();
        std::string consumeBodyChunk();
};