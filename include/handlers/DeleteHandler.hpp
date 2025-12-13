#ifndef DELETEHANDLER_HPP
#define DELETEHANDLER_HPP

#include "../http/HttpRequest.hpp"
#include "../http/HttpResponse.hpp"
#include "../config/Location.hpp"
#include "../core/ServerConfig.hpp"
#include "../../include/interfaces/IMethodHandler.hpp"

class DeleteHandler: public IMethodHandler {

    private:
        const ServerConfig& _config;
        const HttpRequest& _request;
        const Location& _location;
        HttpResponse* _response;
        bool _isFinish;
        DeleteHandler();
        DeleteHandler& operator=(const DeleteHandler &other);
    public:
        DeleteHandler(const ServerConfig& config, const HttpRequest& request, const Location& location);
        DeleteHandler(const DeleteHandler &other);
        HttpResponse& process(const HttpRequest &request, const Location &location);
        virtual ~DeleteHandler();
        virtual void handleData(const std::string& chunk);
        virtual bool isFinished();
        virtual HttpResponse& getResponse();
        virtual IMethodHandler* clone() const;
};

#endif
