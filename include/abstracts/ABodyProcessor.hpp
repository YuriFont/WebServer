#ifndef ABODYPROCESSOR_HPP
#define ABODYPROCESSOR_HPP

#include "../interfaces/IBodyProcessor.hpp"
#include "../core/ServerConfig.hpp"

/**
  @class ABodyProcessor
  @brief Base class for request body processors.
 
  Concrete implementations can handle:

    - Upload to file / RawProcessor
    
    -  x-www-form-urlencoded / UrlEncodedProcessor

    - Multipart/form-data / MultipartProcessor
 */
class ABodyProcessor : public IBodyProcessor {

    protected:
        const ServerConfig &_config;
        static unsigned long _uploadCounter;
        bool _isFinished;
        ABodyProcessor(const ServerConfig &config);
    public:
        virtual ~ABodyProcessor() {};
        virtual bool isMaxBodySize() = 0;
};

#endif