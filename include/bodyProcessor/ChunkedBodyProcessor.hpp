#pragma once

#include <fstream>
#include <string>
#include "../abstracts/ABodyProcessor.hpp"

class ChunkedBodyProcessor : public ABodyProcessor {

public:
    ChunkedBodyProcessor(
        const ServerConfig& config,
        const Location& location,
        const HttpRequest& request
    );

    ~ChunkedBodyProcessor();

    // IBodyProcessor
    void handleChunk(const std::string& chunk);
    bool isFinished();
    HttpResponse* getResult();

    // ABodyProcessor
    bool isMaxBodySize();

private:
    enum State {
        READ_CHUNK_SIZE,
        READ_CHUNK_DATA,
        READ_CRLF,
        DONE,
        ERROR
    };

    State       _state;
    std::string _buffer;

    size_t      _currentChunkSize;
    size_t      _bytesReadInChunk;
    size_t      _totalBytes;

    std::ofstream _file;
    std::string   _filePath;
};
