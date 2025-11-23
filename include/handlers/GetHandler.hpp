#ifndef GETHANDLER_HPP
#define GETHANDLER_HPP

#include "../http/HttpRequest.hpp"
#include "../http/HttpResponse.hpp"
#include "../config/Location.hpp"

class GetHandler {
    private:
        static HttpResponse isDir(const std::string& path, const HttpRequest &request, 
                            const Location &location, struct stat info, HttpResponse response);
    public:
        static HttpResponse process(HttpRequest &request, const Location &location);
};

#endif
