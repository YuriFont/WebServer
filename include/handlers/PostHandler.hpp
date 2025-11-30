#ifndef POSTHANDLER_HPP
#define POSTHANDLER_HPP

#include "../http/HttpRequest.hpp"
#include "../http/HttpResponse.hpp"
#include "../config/Location.hpp"
#include <map>

class PostHandler {

    private:

        typedef struct contentDisposition {
            std::string disposition;
            std::string name;
            std::string fileName;
            std::string contentType;
        } content;

        static std::map<std::string, std::string> _types;
        PostHandler();
        static void initTypes();
        static std::string getExtension(const std::string& type);
        static void saveFile(const std::string& contentType, const std::string& uploadStore, const std::string& body);
        static std::string getBoundary(const std::string& contentType);
        static void processMuiltPart(const std::string& contentType, const std::string& body, const std::string& uploadPath);
        static void getContents(const std::string& line, content& contents);
        static void parseHeaderLine(const std::string& line, content& result);
        static std::string nextLine(const std::string& body, size_t& rangeStart, size_t& endLine);
    public:
        static HttpResponse process(HttpRequest &request, const Location &location);
};

#endif
