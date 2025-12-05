#pragma once

#include "../http/HttpRequest.hpp"
#include "../http/HttpResponse.hpp"
#include "../config/Location.hpp"
#include "../config/Config.hpp"
#include "../interfaces/IMethodHandler.hpp"

class RedirectHandler: public IMethodHandler {

    private:

        const Config& _config;
        const HttpRequest& _request;
        const Location& _location;
        HttpResponse* _response;
        std::string _body;
        bool _isFinish;
        RedirectHandler();
        RedirectHandler& operator=(const RedirectHandler& other);
    public:

        RedirectHandler(const Config& config, const HttpRequest& request, const Location& location);
        RedirectHandler(const RedirectHandler& other);
        virtual ~RedirectHandler();
        virtual void handleData(const std::string& chunk);
        virtual bool isFinished();
        virtual HttpResponse& getResponse();
        virtual IMethodHandler* clone() const;
};