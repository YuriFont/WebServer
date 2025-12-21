#include "../../include/bodyProcessor/MultipartProcessor.hpp"


MultipartProcessor::MultipartProcessor(const ServerConfig& config, const Location& location, const HttpRequest& request): ABodyProcessor::ABodyProcessor(config), _location(location), _bytesReceived(0), _response(NULL), _currentState(READING_HEADERS)  {

    _contentType = request.getHeader("Content-Type");
    _boundary = getBoundary(_contentType);
    _endBoundary = _boundary + "--";
    _contentLength = request.getContentLength();
    if (request.isChunked()) {
        _contentLength = 0; // invalida explicitamente
    }
};

bool MultipartProcessor::isMaxBodySize() {
    if (_contentLength > _config.client_max_body_size)
        return true;
    //else if (_bytesReceived > _contentLength )
    //    return true;
    return false;
};

MultipartProcessor::~MultipartProcessor() {

    if (_response != NULL)
        delete _response;
};

void MultipartProcessor::handleChunk(const std::string& chunk) {

    if (isMaxBodySize()) {
        _response = new HttpResponse();
        _isFinished = true;
        _response->setStatus(413);
        _response->setConnectionClose(true);
        return;
    }
    handleMultipart(chunk);
    //if (_bytesReceived >= _contentLength) {
    //    _response = new HttpResponse();
    //    _response->setStatus(201);
    //    _response->setBody("<h2>Created</h2>");
    //    _isFinished = true;
    //}
};

bool MultipartProcessor::isFinished() {
    return _isFinished;
};

HttpResponse* MultipartProcessor::getResult() {
    return new HttpResponse(*_response);
};

bool MultipartProcessor::append(std::string data, size_t len) {
    
    if (!_outFile.is_open() || !_outFile.good()) 
        return false;
    _outFile.write(data.c_str(), len);
    if (!_outFile) 
        return false;
    return true;
}

std::string MultipartProcessor::setTimeInNameFile(std::string & fileName) {

    std::string result;
    const std::time_t now = std::time(0);

    size_t dot = fileName.rfind('.');
    std::string base;
    std::string ext;

    if (dot != std::string::npos) {
        base = fileName.substr(0, dot);
        ext  = fileName.substr(dot);
    } else {
        base = fileName;
    }

    result.reserve(base.size() + ext.size() + 32);
    result.append(base);
    result.append("_");
    result.append(Utils::toString(now));
    result.append("_");
    result.append(Utils::toString(_uploadCounter++));
    result.append(ext);

    return result;
}


std::string MultipartProcessor::getNameFileMultipart() {

    const std::string uploadStore = _location.getUploadStore() + "/";
    const std::time_t now = std::time(0);

    std::string fileName;

    if (_currentContents.fileName.empty()) {
        fileName = _currentContents.name + ".txt";
    } else {
        fileName = _currentContents.fileName;
    }

    std::string filePath = uploadStore + fileName;

    if (access(filePath.c_str(), F_OK) == 0) {
        if (_currentContents.fileName.empty()) {
            filePath = uploadStore
                     + _currentContents.name
                     + "_"
                     + Utils::toString(now)
                     + "_"
                     + Utils::toString(_uploadCounter++)
                     + ".txt";
        } else {
            filePath = uploadStore + setTimeInNameFile(fileName);
        }
    }

    return filePath;
}

void MultipartProcessor::handleMultipart(const std::string& chunk) {

    // Acumula dados recebidos (stream)
    _body.append(chunk);
    _bytesReceived += chunk.size();

    while (true) {

        /* ===============================
         * ESTADO: READING_HEADERS
         * =============================== */
        if (_currentState == READING_HEADERS) {

            size_t endHeaders = _body.find("\r\n\r\n");
            if (endHeaders == std::string::npos)
                return; // headers ainda incompletos

            std::string headers = _body.substr(0, endHeaders);
            _body.erase(0, endHeaders + 4);

            // Parse dos headers da parte
            _currentContents = content(); // zera struct
            parseHeaderLine(headers, _currentContents);

            // Abre arquivo para escrita
            std::string filePath = getNameFileMultipart();
            _outFile.open(filePath.c_str(), std::ios::binary);
            if (!_outFile.is_open()) {
                _currentState = READY;
                _isFinished = true;
                return;
            }

            _currentState = READING_BODY;
        }

        /* ===============================
         * ESTADO: READING_BODY
         * =============================== */
        if (_currentState == READING_BODY) {

            size_t boundaryPos = _body.find(_boundary);

            if (boundaryPos == std::string::npos) {
                // Não encontramos boundary ainda
                // Escrevemos tudo menos o possível prefixo da boundary
                size_t safeSize = _body.size() > _boundary.size()
                                ? _body.size() - _boundary.size()
                                : 0;

                if (safeSize > 0) {
                    _outFile.write(_body.data(), safeSize);
                    _body.erase(0, safeSize);
                }
                return;
            }

            // Encontramos boundary → final do arquivo atual
            size_t bytesToWrite = boundaryPos;

            // Remove CRLF antes da boundary
            if (bytesToWrite >= 2 &&
                _body.substr(bytesToWrite - 2, 2) == "\r\n") {
                bytesToWrite -= 2;
            }

            // Escreve dados finais do arquivo
            _outFile.write(_body.data(), bytesToWrite);
            _outFile.close();

            // Avança buffer até depois da boundary
            size_t afterBoundary = boundaryPos + _boundary.size();
            bool isFinalBoundary = false;

            if (_body.substr(afterBoundary, 2) == "--") {
                isFinalBoundary = true;
                afterBoundary += 2;
            }

            // Consome CRLF após boundary
            if (_body.substr(afterBoundary, 2) == "\r\n")
                afterBoundary += 2;

            _body.erase(0, afterBoundary);

            if (isFinalBoundary) {
                // 🔥 FINAL REAL DO MULTIPART
                _currentState = READY;
                _isFinished = true;

                _response = new HttpResponse();
                _response->setStatus(201);
                _response->setBody("<h2>Created</h2>");
                return;
            }

            // Próxima parte
            _currentState = READING_HEADERS;
            continue;
        }

        return;
    }
}


// bool MultipartProcessor::append(std::string data, size_t len) {};

void MultipartProcessor::createFile() {};

void MultipartProcessor::finishFileUpload() {};

void MultipartProcessor::parseHeaderLine(const std::string& line, content& result) {
    // Caso 1: Content-Disposition
    if (line.find("Content-Disposition:") != std::string::npos) {
        size_t start = line.find(": ");
        size_t end = line.find(";");
        
        if (start != std::string::npos && end != std::string::npos) {
            // +2 para pular ": "
            result.disposition = line.substr(start + 2, end - (start + 2));
        }

        result.name = Utils::extractValue(line, "name=\"");
        result.fileName = Utils::extractValue(line, "filename=\"");
    }
    // Caso 2: Content-Type
    if (line.find("Content-Type:") != std::string::npos) {
        size_t start = line.find(": ");
        if (start != std::string::npos) {
            // Pega tudo do ": " até o fim da string (ou remove espaços extras se necessário)
            std::string type = line.substr(start + 2);
            
            // Opcional: Remover \r ou \n se a string vier bruta do socket
            size_t endClean = type.find_last_not_of(" \t\r\n");
            if (endClean != std::string::npos)
                 result.contentType = type.substr(0, endClean + 1);
            else 
                 result.contentType = type;
        }
    }
}

std::string MultipartProcessor::getBoundary(const std::string& contentType) {

    size_t pos = contentType.find("boundary=");
    if (pos == std::string::npos) {
        std::cout << "Errou" << std::endl;
    }
    pos += 9;
    std::string boundary = contentType.substr(pos);
    boundary =  "--" + boundary;

    return boundary;
}