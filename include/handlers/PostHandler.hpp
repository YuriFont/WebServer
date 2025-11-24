#ifndef POSTHANDLER_HPP
#define POSTHANDLER_HPP

#include "../http/HttpRequest.hpp"
#include "../http/HttpResponse.hpp"
#include "../config/Location.hpp"

class PostHandler {
    private:
        static bool isDocFile(const std::string& contentType);
        static std::string getImageExtension(const std::string& typeImage);
        static std::string getAudioExtension(const std::string& typeAudio);
        static std::string getVideoExtension(const std::string& typeVideo);
        static std::string getDocExtension(const std::string& typeDoc);
        static void saveAsAudio(const std::string& typeAudio, const std::string& uploadStore, const std::string& body);
        static void saveAsVideo(const std::string& typeVideo, const std::string& uploadStore, const std::string& body);
        static void saveAsDoc(const std::string& typeDoc, const std::string& uploadStore, const std::string& body);
        static void saveAsImage(const std::string& typeImage, const std::string& uploadStore, const std::string& body);
    public:
        static HttpResponse process(HttpRequest &request, const Location &location);
};

#endif
