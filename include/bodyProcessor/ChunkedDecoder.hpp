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
            READ_SIZE,
            READ_SIZE_CR,
            READ_DATA,
            READ_DATA_CR,
            READ_DATA_LF,
            READ_FINAL_CR,
            READ_FINAL_LF,
            DONE,
            ERROR
        };

        State _state;
        size_t _currentChunkSize;
        size_t _bytesReadInChunk;

        std::string _sizeBuffer;
        std::string _body;
};