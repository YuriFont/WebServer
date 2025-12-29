#include "../../include/bodyProcessor/BodyProcessorFactory.hpp"


ABodyProcessor* BodyProcessorFactory::createBodyProcessor(const ServerConfig& config, const Location& location, const HttpRequest& request) {
    if (request.getHeader("Content-Type") == "application/x-www-form-urlencoded")
        return new UrlEncodedProcessor(config, location, request);
    else if (request.getHeader("Content-Type").find("multipart/form-data") != std::string::npos)
        return new MultipartProcessor(config, location, request);
    else
        return new RawProcessor(config, location, request);
};