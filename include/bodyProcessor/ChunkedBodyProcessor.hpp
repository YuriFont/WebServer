#pragma once

#include <fstream>
#include <string>

#include "../abstracts/ABodyProcessor.hpp"
#include "../config/Location.hpp"
#include "../http/HttpRequest.hpp"

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

    enum ErrorReason {
        NO_ERROR,
        BAD_CHUNK_FORMAT,     // 400
        BODY_TOO_LARGE,       // 413
        FILE_IO_ERROR         // 500 (opcional, mas faz sentido)
    };

private:
    bool openTempFile(const Location& location, const HttpRequest& request);
    bool parseHexSizeLine(const std::string& hexLine, size_t& outSize);

private:
    State        _state;
    ErrorReason  _error;

    std::string  _buffer;

    size_t       _currentChunkSize;
    size_t       _bytesReadInChunk;
    size_t       _totalBytes;

    std::ofstream _file;
    std::string   _filePath;
};
