#ifndef MULTIPARTPROCESSOR_HPP
#define MULTIPARTPROCESSOR_HPP

#include "../abstracts/ABodyProcessor.hpp"
#include "../config/Location.hpp"
#include "../http/HttpRequest.hpp"
#include "../http/HttpResponse.hpp"
#include "../utils/Utils.hpp"
#include <fstream>
#include <string>
#include <iostream>

class MultipartProcessor : public ABodyProcessor {

    private:
        MultipartProcessor();
        const Location& _location;
        std::string _contentType;
        std::string _fileName;
        std::string _boundary;
        std::string _endBoundary;
        size_t _contentLength;
        size_t _bytesReceived;
        std::ofstream _outFile;
        HttpResponse* _response;
        std::string _body;

        enum State {
            READING_HEADERS,
            READING_BODY,
            READY
        };

        typedef struct contentDisposition {
            std::string disposition;
            std::string name;
            std::string fileName;
            std::string contentType;
        } content;

        State _currentState;
        content _currentContents;
    
    public:

        MultipartProcessor(const ServerConfig& config, const Location& location, const HttpRequest& request);
        virtual ~MultipartProcessor();
        virtual void handleChunk(const std::string& chunk);
        virtual bool isFinished();
        virtual HttpResponse* getResult();
        virtual bool isMaxBodySize();
        void handleMultipart(const std::string& chunk);
        // void saveFile(const std::string& contentType, const std::string& uploadStore, const std::string& body);
        bool append(std::string data, size_t len);
        void parseHeaderLine(const std::string& line, content& result);
        void createFile();
        void finishFileUpload();
        std::string getBoundary(const std::string& contentType);
};

#endif