#ifndef REQUESTHANDLER_HPP
#define REQUESTHANDLER_HPP

#include "../config/Config.hpp"
#include "../http/HttpRequest.hpp"
#include "../http/HttpResponse.hpp"
#include "CgiHandler.hpp"

class IBodyProcessor {

    public:
        virtual ~IBodyProcessor() {};
        virtual void handleChunk(const std::string& chunk) = 0;
        virtual bool isFinished() = 0;
        virtual std::string getResult() = 0;
        IBodyProcessor(const Config &config) = 0;
        
    private:
        const Config &_config;
};

#endif
