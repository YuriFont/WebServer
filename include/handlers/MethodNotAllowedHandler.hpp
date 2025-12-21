#ifndef METHODNOTALLOWEDHANDLER_HPP
#define METHODNOTALLOWEDHANDLER_HPP

#include "../http/HttpRequest.hpp"
#include "../http/HttpResponse.hpp"
#include "../config/Location.hpp"
#include "../core/ServerConfig.hpp"
#include "../interfaces/IMethodHandler.hpp"
#include <vector>

class MethodNotAllowedHandler : public IMethodHandler {

    private:
        std::string _methodsAllowed;
        HttpResponse *_response;
        MethodNotAllowedHandler();
    public:
        ~MethodNotAllowedHandler();
        MethodNotAllowedHandler(const MethodNotAllowedHandler& other);
        MethodNotAllowedHandler(const std::vector<std::string>& alloweds);
        virtual void handleData(const std::string& chunk);
        virtual bool isFinished();
        virtual HttpResponse& getResponse();
        virtual IMethodHandler* clone() const;
};

#endif