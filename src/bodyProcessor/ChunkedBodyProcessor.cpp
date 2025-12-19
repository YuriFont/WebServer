#include "../../include/bodyProcessor/ChunkedBodyProcessor.hpp"
#include "../../include/utils/Utils.hpp"

#include <cstdlib>
#include <ctime>
#include <cerrno>
#include <iostream>   // prints
#include <cstdio>     // std::remove

ChunkedBodyProcessor::ChunkedBodyProcessor(
    const ServerConfig& config,
    const Location& location,
    const HttpRequest& request
)
: ABodyProcessor(config),
  _state(READ_CHUNK_SIZE),
  _error(NO_ERROR),
  _currentChunkSize(0),
  _bytesReadInChunk(0),
  _totalBytes(0)
{
    if (!openTempFile(location, request)) {
        _state = ERROR;
        _error = FILE_IO_ERROR;
        std::cerr << "[ChunkedBodyProcessor] ERROR: failed to open temp file\n";
        return;
    }
}

ChunkedBodyProcessor::~ChunkedBodyProcessor() {
    if (_file.is_open())
        _file.close();
}

bool ChunkedBodyProcessor::openTempFile(const Location& location, const HttpRequest& request) {
    (void)request;

    std::srand(std::time(NULL));
    _filePath = location.getUploadStore()
              + "/chunked_"
              + Utils::toString(std::time(NULL))
              + "_"
              + Utils::toString(std::rand())
              + ".txt";

    _file.open(_filePath.c_str(), std::ios::binary | std::ios::out);
    if (!_file.is_open())
        return false;

    // Se você estiver usando request.bodyTempPath no seu projeto:
    // const_cast<HttpRequest&>(request).bodyTempPath = _filePath;

    return true;
}

// Converte linha hex (ex: "4", "1A", "8000") para size_t.
// Aceita também extensões de chunk ("4;ext=1") -> pega só antes do ';'
bool ChunkedBodyProcessor::parseHexSizeLine(const std::string& hexLine, size_t& outSize) {

    std::string clean = hexLine;

    // corta extensões: "4;ext=abc" -> "4"
    size_t semi = clean.find(';');
    if (semi != std::string::npos)
        clean = clean.substr(0, semi);

    clean = Utils::trim(clean);
    if (clean.empty())
        return false;

    char* endptr = NULL;
    unsigned long val = std::strtoul(clean.c_str(), &endptr, 16);

    if (endptr == NULL || *endptr != '\0')
        return false;

    outSize = static_cast<size_t>(val);
    return true;
}

void ChunkedBodyProcessor::handleChunk(const std::string& chunk) {

    if (_state == DONE || _state == ERROR)
        return;

    _buffer.append(chunk);

    while (true) {

        // ================= READ CHUNK SIZE =================
        if (_state == READ_CHUNK_SIZE) {
            size_t pos = _buffer.find("\r\n");
            if (pos == std::string::npos) {
                // ainda não chegou a linha completa
                return;
            }

            std::string line = _buffer.substr(0, pos);
            _buffer.erase(0, pos + 2);

            size_t parsedSize = 0;
            if (!parseHexSizeLine(line, parsedSize)) {
                _state = ERROR;
                _error = BAD_CHUNK_FORMAT;
                std::cerr << "[ChunkedBodyProcessor] ERROR: bad chunk size line: '" << line << "'\n";
                return;
            }

            _currentChunkSize = parsedSize;
            _bytesReadInChunk = 0;

            // chunk size 0 = fim. Ainda pode vir trailers + "\r\n".
            if (_currentChunkSize == 0) {
                // RFC: depois de "0\r\n" pode vir trailers até "\r\n\r\n".
                // Como seu servidor não usa trailers, o mínimo esperado é "\r\n".
                // Vamos consumir opcionalmente um "\r\n" se já estiver no buffer.
                if (_buffer.size() >= 2 && _buffer.substr(0, 2) == "\r\n")
                    _buffer.erase(0, 2);

                _state = DONE;
                return;
            }

            _state = READ_CHUNK_DATA;
        }

        // ================= READ CHUNK DATA =================
        if (_state == READ_CHUNK_DATA) {
            size_t remaining = _currentChunkSize - _bytesReadInChunk;

            if (_buffer.size() < remaining) {
                // não chegou tudo ainda
                return;
            }

            // antes de escrever: checa se estouraria limite
            if (_totalBytes + remaining > _config.client_max_body_size) {
                _state = ERROR;
                _error = BODY_TOO_LARGE;
                std::cerr << "[ChunkedBodyProcessor] ERROR: body too large (limit="
                          << _config.client_max_body_size << ")\n";
                return;
            }

            _file.write(_buffer.c_str(), remaining);
            if (!_file.good()) {
                _state = ERROR;
                _error = FILE_IO_ERROR;
                std::cerr << "[ChunkedBodyProcessor] ERROR: file write failed\n";
                return;
            }

            _buffer.erase(0, remaining);

            _bytesReadInChunk += remaining;
            _totalBytes += remaining;

            _state = READ_CRLF;
        }

        // ================= READ CRLF =================
        if (_state == READ_CRLF) {
            if (_buffer.size() < 2)
                return;

            if (_buffer.substr(0, 2) != "\r\n") {
                _state = ERROR;
                _error = BAD_CHUNK_FORMAT;
                std::cerr << "[ChunkedBodyProcessor] ERROR: missing CRLF after chunk data\n";
                return;
            }

            _buffer.erase(0, 2);
            _state = READ_CHUNK_SIZE;

            // continua no loop pra tentar processar próximo chunk (se já tiver)
            continue;
        }

        return;
    }
}

bool ChunkedBodyProcessor::isFinished() {
    return (_state == DONE || _state == ERROR);
}

bool ChunkedBodyProcessor::isMaxBodySize() {
    return (_totalBytes > _config.client_max_body_size);
}

HttpResponse* ChunkedBodyProcessor::getResult() {

    HttpResponse* res = new HttpResponse();

    // 🔴 PRIORIDADE ABSOLUTA
    if (_error == BODY_TOO_LARGE) {
        res->setStatus(413);
        res->setBody("<h1>413 Payload Too Large</h1>");
        res->setConnectionClose(true);
        return res;
    }

    if (_state == ERROR) {
        res->setStatus(400);
        res->setBody("<h1>400 Bad Request</h1>");
        res->setConnectionClose(true);
        return res;
    }

    res->setStatus(201);
    res->setBody("<h1>Chunked upload completed</h1>");
    res->setConnectionClose(true);
    return res;
}

