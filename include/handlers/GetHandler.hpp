#ifndef GETHANDLER_HPP
#define GETHANDLER_HPP

#include "../http/HttpRequest.hpp"
#include "../http/HttpResponse.hpp"
#include "../config/Location.hpp"
#include "../config/Config.hpp"
#include "../interfaces/IMethodHandler.hpp"

class GetHandler : public IMethodHandler {

    private:

        const Config& _config;
        const HttpRequest& _request;
        const Location& _location;
        HttpResponse* _response;
        void isDir(const std::string& path, const HttpRequest &request, 
                            const Location &location, struct stat info, HttpResponse& response);
        GetHandler();
        GetHandler& operator=(const GetHandler& other);
    public:
        GetHandler(const Config& config, const HttpRequest& request, const Location& location);
        GetHandler(const GetHandler& other);
        ~GetHandler();
        virtual void handleData(const std::string& chunk);
        virtual bool isFinished();
        virtual HttpResponse& getResponse();
        virtual IMethodHandler* clone() const;
        HttpResponse& process(const HttpRequest &request, const Location &location);
};

#endif
