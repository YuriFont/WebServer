#pragma once

#include <map>
#include <utility>
#include <string>

class HttpStatus {
    
    private:
    
        std::map<int, std::string> httpStatusMap;
    public:
        HttpStatus();
        ~HttpStatus();
        const std::string& getHttpStatusMensager(const int& status) const;
};