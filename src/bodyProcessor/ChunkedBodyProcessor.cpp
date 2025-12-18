#include "../../include/bodyProcessor/ChunkedBodyProcessor.hpp"
#include "../../include/utils/Utils.hpp"
#include <cstdlib>
#include <ctime>

ChunkedBodyProcessor::ChunkedBodyProcessor(
    const ServerConfig& config,
    const Location& location,
    const HttpRequest& request
)
: ABodyProcessor(config),
  _state(READ_CHUNK_SIZE),
  _currentChunkSize(0),
  _bytesReadInChunk(0),
  _totalBytes(0)
{
    std::srand(std::time(NULL));
    _filePath = location.getUploadStore()
              + "/chunked_"
              + Utils::toString(std::time(NULL))
              + "_"
              + Utils::toString(std::rand())
              + ".tmp";

    _file.open(_filePath.c_str(), std::ios::binary | std::ios::out);
    if (!_file.is_open()) {
        _state = ERROR;
        return;
    }

    // 🔹 REGISTRA O ARQUIVO TEMP NO REQUEST
    const_cast<HttpRequest&>(request).bodyTempPath = _filePath;
}

ChunkedBodyProcessor::~ChunkedBodyProcessor() {
    if (_file.is_open())
        _file.close();
}

void ChunkedBodyProcessor::handleChunk(const std::string& chunk) {

    if (_state == DONE || _state == ERROR)
        return;

    _buffer.append(chunk);

    while (true) {

        // ================= READ CHUNK SIZE =================
        if (_state == READ_CHUNK_SIZE) {
            size_t pos = _buffer.find("\r\n");
            if (pos == std::string::npos)
                return;

            std::string hex = _buffer.substr(0, pos);
            _buffer.erase(0, pos + 2);

            char* endptr;
            _currentChunkSize = std::strtoul(hex.c_str(), &endptr, 16);
            if (*endptr != '\0') {
                _state = ERROR;
                return;
            }

            _bytesReadInChunk = 0;

            if (_currentChunkSize == 0) {
                _state = DONE;
                return;
            }

            _state = READ_CHUNK_DATA;
        }

        // ================= READ CHUNK DATA =================
        if (_state == READ_CHUNK_DATA) {
            size_t remaining = _currentChunkSize - _bytesReadInChunk;
            if (_buffer.size() < remaining)
                return;

            _file.write(_buffer.c_str(), remaining);
            if (!_file.good()) {
                _state = ERROR;
                return;
            }

            _buffer.erase(0, remaining);
            _bytesReadInChunk += remaining;
            _totalBytes += remaining;

            if (_totalBytes > _config.client_max_body_size) {
                _state = ERROR;
                return;
            }

            _state = READ_CRLF;
        }

        // ================= READ CRLF =================
        if (_state == READ_CRLF) {
            if (_buffer.size() < 2)
                return;

            if (_buffer.substr(0, 2) != "\r\n") {
                _state = ERROR;
                return;
            }

            _buffer.erase(0, 2);
            _state = READ_CHUNK_SIZE;
        }
    }
}

bool ChunkedBodyProcessor::isFinished() {
    return _state == DONE || _state == ERROR;
}

bool ChunkedBodyProcessor::isMaxBodySize() {
    return _totalBytes > _config.client_max_body_size;
}

HttpResponse* ChunkedBodyProcessor::getResult() {

    HttpResponse* res = new HttpResponse();

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
