#pragma once

#include <string>
#include "../http/HttpResponse.hpp"

class IMethodHandler {

    private:
        bool _isError;

    public:
        IMethodHandler() : _isError(false) {};
        virtual ~IMethodHandler() {};
        virtual void handleData(const std::string& chunk) = 0;
        virtual bool isFinished() = 0;
        virtual HttpResponse& getResponse() = 0;
        virtual IMethodHandler* clone() const = 0;
        virtual bool isCgi() const { return false; };

        virtual bool isError() {
            return this->_isError;
        }

        virtual void setIsError(const bool& value) {
            this->_isError = value;
        }
};