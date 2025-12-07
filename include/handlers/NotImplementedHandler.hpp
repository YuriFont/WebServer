#pragma once

#include "../http/HttpRequest.hpp"
#include "../http/HttpResponse.hpp"
#include "../config/Location.hpp"
#include "../core/ServerConfig.hpp"
#include "../interfaces/IMethodHandler.hpp"

class NotImplementedHandler : public IMethodHandler {

    private:

        const ServerConfig& _config;
        const HttpRequest& _request;
        const Location& _location;
        HttpResponse* _response;
        bool _isFinish;
        NotImplementedHandler();
        NotImplementedHandler& operator=(const NotImplementedHandler& other);
    public:
        NotImplementedHandler(const ServerConfig& config, const HttpRequest& request, const Location& location);
        NotImplementedHandler(const NotImplementedHandler& other);
        ~NotImplementedHandler();
        virtual void handleData(const std::string& chunk);
        virtual bool isFinished();
        virtual HttpResponse& getResponse();
        virtual IMethodHandler* clone() const;

};