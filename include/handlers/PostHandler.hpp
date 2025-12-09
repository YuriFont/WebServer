#ifndef POSTHANDLER_HPP
#define POSTHANDLER_HPP

#include "../http/HttpRequest.hpp"
#include "../http/HttpResponse.hpp"
#include "../config/Location.hpp"
#include "../core/ServerConfig.hpp"
#include "../interfaces/IMethodHandler.hpp"
#include "../abstracts/ABodyProcessor.hpp"
#include "../bodyProcessor/RawProcessor.hpp"
#include "../bodyProcessor/MultipartProcessor.hpp"
#include "../bodyProcessor/UrlEncodedProcessor.hpp"
#include <map>

class PostHandler : public IMethodHandler {

    private:

        const ServerConfig& _config;
        const HttpRequest& _request;
        const Location& _location;
        HttpResponse* _response;
        ABodyProcessor* _bodyProcessor;
        std::string _body;
        bool _isFinish;

        typedef struct contentDisposition {
            std::string disposition;
            std::string name;
            std::string fileName;
            std::string contentType;
        } content;
        static std::map<std::string, std::string> _types;
        static void initTypes();

        PostHandler();
        PostHandler& operator=(const PostHandler& other);

        std::string getExtension(const std::string& type);
        void saveFile(const std::string& contentType, const std::string& uploadStore, const std::string& body);
        std::string getBoundary(const std::string& contentType);
        void processMuiltPart(const std::string& contentType, const std::string& body, const std::string& uploadPath);
        void getContents(const std::string& line, content& contents);
        void parseHeaderLine(const std::string& line, content& result);
        std::string nextLine(const std::string& body, size_t& rangeStart, size_t& endLine);
        void handleRawPost(const Location& location, const std::string contentType);
        void handleMultipart(const Location& location, HttpResponse& response, const std::string& body, const std::string contentType);
        void handleFormUrlencoded(const Location& location, HttpResponse& response, const std::string& body);
    public:
        HttpResponse& process(const HttpRequest &request, const Location &location);
        PostHandler(const ServerConfig& config, const HttpRequest& request, const Location& location);
        PostHandler(const PostHandler& other);
        virtual ~PostHandler();
        virtual void handleData(const std::string& chunk);
        virtual bool isFinished();
        virtual HttpResponse& getResponse();
        virtual IMethodHandler* clone() const;
};

#endif
