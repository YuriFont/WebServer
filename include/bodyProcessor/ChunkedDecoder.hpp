#pragma once

#include "../core/WebServer.hpp"

class ChunkedDecoder {
    public:
        ChunkedDecoder();

        void reset();
        void feed(const char* data, size_t len);
        bool isFinished() const;
        const std::string& getBody() const;

    private:
        enum State {
            READ_SIZE, //0
            READ_DATA, //1
            READ_CRLF, //2
            DONE //3
        };

        State _state;
        size_t _currentChunkSize;
        size_t _bytesReadInChunk;

        std::string _sizeBuffer;
        std::string _body;
};