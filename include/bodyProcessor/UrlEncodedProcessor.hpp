#ifndef URLENCODEDPROCESSOR_HPP
#define URLENCODEDPROCESSOR_HPP

#include "../abstracts/ABodyProcessor.hpp"
#include "../config/Location.hpp"
#include "../http/HttpRequest.hpp"
#include "../http/HttpResponse.hpp"
#include "../utils/Utils.hpp"
#include <fstream>
#include <string>
#include <iostream>

class UrlEncodedProcessor : public ABodyProcessor {

    private:
        UrlEncodedProcessor();
        const Location& _location;
        std::string _contentType;
        std::string _fileName;
        size_t _contentLength;
        size_t _bytesReceived;
        std::ofstream _outFile;
        HttpResponse* _response;
        std::string _body;


    public:

        UrlEncodedProcessor(const ServerConfig& config, const Location& location, const HttpRequest& request);
        virtual ~UrlEncodedProcessor();
        virtual void handleChunk(const std::string& chunk);
        virtual bool isFinished();
        virtual HttpResponse* getResult();
        virtual bool isMaxBodySize();
        void handleFormUrlencoded(const std::string& body);

};

#endif