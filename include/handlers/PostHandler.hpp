#ifndef POSTHANDLER_HPP
#define POSTHANDLER_HPP

#include "../http/HttpRequest.hpp"
#include "../http/HttpResponse.hpp"
#include "../config/Location.hpp"
#include <map>

class PostHandler {

    private:

        static std::map<std::string, std::string> _types;
        PostHandler();
        static void initTypes();
        static std::string getExtension(const std::string& type);
        static void saveFile(const std::string& contentType, const std::string& uploadStore, const std::string& body);
    public:
        static HttpResponse process(HttpRequest &request, const Location &location);
};

#endif
