#ifndef IBODYPROCESSOR_HPP
#define IBODYPROCESSOR_HPP

#include "../core/ServerConfig.hpp"
#include "../http/HttpRequest.hpp"
#include "../http/HttpResponse.hpp"

class IBodyProcessor {

    private:

    public:
        virtual ~IBodyProcessor() {};
        virtual void handleChunk(const std::string& chunk) = 0;
        virtual bool isFinished() = 0;
        virtual HttpResponse* getResult() = 0;
        
};

#endif
