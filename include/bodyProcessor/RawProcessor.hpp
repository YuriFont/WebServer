#ifndef RAWPROCESSOR_HPP
#define RAWPROCESSOR_HPP

#include "../abstracts/ABodyProcessor.hpp"
#include "../config/Location.hpp"
#include "../http/HttpRequest.hpp"
#include "../http/HttpResponse.hpp"
#include <map>
#include <fstream>
#include <string>


class RawProcessor : public ABodyProcessor {
    private:
        RawProcessor();
        const Location& _location;
        std::string _contentType;
        std::string _fileName;
        size_t _contentLength;
        size_t _bytesReceived;
        std::ofstream _outFile;
        HttpResponse* _response; 
        static std::map<std::string, std::string> _types;
        static void initTypes();
    
    public:

        RawProcessor(const ServerConfig& config, const Location& location, const HttpRequest& request);
        virtual ~RawProcessor();
        virtual void handleChunk(const std::string& chunk);
        virtual bool isFinished();
        virtual HttpResponse* getResult();
        virtual bool isMaxBodySize();
        std::string getExtension(const std::string& contentType);
        void handleRawPost(const std::string& chunk);
        void saveFile(const std::string& contentType, const std::string& uploadStore, const std::string& body);
        bool append(std::string data, size_t len);
        void createFile();
        void finishFileUpload();

};

#endif