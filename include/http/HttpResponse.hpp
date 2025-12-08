#pragma once

#include "../http/HttpStatus.hpp"
#include <vector>

class HttpResponse {
    
    // versão http, codigo e mensagem do resultado
    private:

        HttpStatus status;
        std::string httpVersion;
        std::string contentType;
        int contentLength;
        std::string body;
        std::string allowedMethods;
        bool isNotAllow;
        bool connectionClose;
        std::map<std::string, std::string> _headers;
        
    public:

        HttpResponse();
        HttpResponse(const HttpResponse& other);
        HttpResponse& operator=(const HttpResponse& other);
        ~HttpResponse();
        void setStatus(const int& statusCode);
        void setHttpVersion(const std::string& httpVersion);
        void setContentType(const std::string& contentType);
        void setContentLength(const int& contentLength);
        void setBody(const std::string& body);
        void setConnectionClose(bool connectionClose);
        bool isConnectionClose();
        int getContentLength();
        static HttpResponse methodNotAllowed(const std::vector<std::string>& methods);
        void setAllowedMethods(const std::vector<std::string>& methods);
        void setHeader(const std::string &key, const std::string &value);
        std::string toString();
};
