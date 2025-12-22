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
#include "../bodyProcessor/BodyProcessorFactory.hpp"
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

        // typedef struct contentDisposition {
        //     std::string disposition;
        //     std::string name;
        //     std::string fileName;
        //     std::string contentType;
        // } content;
        // static std::map<std::string, std::string> _types;
        // static void initTypes();

        PostHandler();
        PostHandler& operator=(const PostHandler& other);

    public:
        PostHandler(const ServerConfig& config, const HttpRequest& request, const Location& location);
        PostHandler(const PostHandler& other);
        virtual ~PostHandler();
        virtual void handleData(const std::string& chunk);
        virtual bool isFinished();
        virtual HttpResponse& getResponse();
        virtual IMethodHandler* clone() const;
};

#endif
