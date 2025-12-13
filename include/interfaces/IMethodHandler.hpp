#pragma once

#include <string>
#include "../http/HttpResponse.hpp"

class IMethodHandler {

    public:
        virtual ~IMethodHandler() {};
        virtual void handleData(const std::string& chunk) = 0;
        virtual bool isFinished() = 0;
        virtual HttpResponse& getResponse() = 0;
        virtual IMethodHandler* clone() const = 0;
};