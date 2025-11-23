#pragma once

#include <map>
#include <utility>
#include <string>

class HttpStatus {
    
    private:
    
        std::map<int, std::string> httpStatusMap;
        int codeStatus;
    public:
        HttpStatus();
        ~HttpStatus();
        const std::string& getHttpStatusMensager(const int& status) const;
        int getCodeStatus();
        void setCodeStatus(const int& codeStatus);
        const std::string makeResponseStatus();
};