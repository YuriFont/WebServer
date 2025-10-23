#pragma once

#include "./HttpStatus.hpp"
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
        
    public:

        HttpResponse();
        void setStatus(const int& statusCode);
        void setHttpVersion(const std::string& httpVersion);
        void setContentType(const std::string& contentType);
        void setContentLength(const int& contentLength);
        void setBody(const std::string& body);
        void setConnectionClose(bool connectionClose);
        int getContentLength();
        static HttpResponse methodNotAllowed(const std::vector<std::string>& methods);
        void setAllowedMethods(const std::vector<std::string>& methods);
        std::string toString();
};