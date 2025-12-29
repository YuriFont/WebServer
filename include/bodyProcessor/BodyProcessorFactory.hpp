#ifndef BODYPROCESSORFACTORY_HPP
#define BODYPROCESSORFACTORY_HPP

#include "./IgnoreProcessor.hpp"
#include "./MultipartProcessor.hpp"
#include "./RawProcessor.hpp"
#include "./UrlEncodedProcessor.hpp"
#include "../abstracts/ABodyProcessor.hpp"
#include "../config/Location.hpp"
#include "../core/ServerConfig.hpp"
#include "../http/HttpRequest.hpp"

class BodyProcessorFactory {

    private:
        BodyProcessorFactory();
        BodyProcessorFactory(const BodyProcessorFactory&);
        BodyProcessorFactory& operator=(const BodyProcessorFactory&);
        ~BodyProcessorFactory();
    public:
        static ABodyProcessor* createBodyProcessor(const ServerConfig& config, const Location& location, const HttpRequest& request);
};

#endif