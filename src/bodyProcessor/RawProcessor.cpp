#include "../../include/bodyProcessor/RawProcessor.hpp"
#include "../../include/utils/Utils.hpp"

std::map<std::string, std::string> RawProcessor::_types;

RawProcessor::RawProcessor(const ServerConfig& config, const Location& location, const HttpRequest& request) : ABodyProcessor::ABodyProcessor(config), _location(location), _bytesReceived(0), _response(NULL) {
    initTypes();
    std::string type = request.getHeader("Content-Type");

    size_t splitType = type.find(";");
    if (splitType != std::string::npos) {
        type = type.substr(0, splitType);
    }
    _contentType = type;
    _contentLength = request.getContentLength();
};

bool RawProcessor::isMaxBodySize() {
    if (_contentLength > _config.client_max_body_size)
        return true;
    else if (_bytesReceived > _config.client_max_body_size)
        return true;
    return false;
};

RawProcessor::~RawProcessor() {

    if (_response != NULL) {
        delete _response;
    }
};

bool RawProcessor::append(std::string data, size_t len) {
    
    if (!_outFile.is_open() || !_outFile.good()) 
        return false;
    _outFile.write(data.c_str(), len);
    if (!_outFile) 
        return false;
    _bytesReceived += len;
    return true;
}

void RawProcessor::createFile() {
    std::string extension = getExtension(_contentType);
    std::string uploadPath = _location.getUploadStore() + "/upload_" + Utils::toString(std::time(0)) + "_" + Utils::toString(_uploadCounter++) + extension;
    _fileName = uploadPath;
    _outFile.open(_fileName.c_str(), std::ios::binary | std::ios::app);
}

void RawProcessor::finishFileUpload() {
    
    _response->setStatus(201);
    _response->setContentType("text/html");
    _response->setBody("Sucess upload file");
    std::cout << "Upload sucessfull in " + _fileName << std::endl;
    _isFinished = true;
    if (_outFile.is_open()) {
        _outFile.flush();
        _outFile.close();
    }
}

void RawProcessor::handleChunk(const std::string& chunk) {

    if (isMaxBodySize()) {
        _response = new HttpResponse();
        _isFinished = true;
        _response->setStatus(413);
        _response->setConnectionClose(true);
        return;
    }
    if (_fileName.empty()) {
        createFile();
    }
    handleRawPost(chunk);
    if (_bytesReceived >= _contentLength) {
        finishFileUpload();
    }
};

bool RawProcessor::isFinished() {
    return (_isFinished);
};

HttpResponse* RawProcessor::getResult() {
    return new HttpResponse(*_response);
};

void RawProcessor::initTypes() {

    if (!_types.empty())
        return;

    _types["image/jpeg"] = ".jpg";
    _types["image/png"]  = ".png";
    _types["image/gif"]  = ".gif";
    _types["image/webp"] = ".webp";
    _types["image/svg+xml"] = ".svg";
    _types["image/bmp"]  = ".bmp";
    _types["image/tiff"] = ".tiff";

    // Adicionando casos curtos se o seu parser enviar apenas o subtipo "jpeg" em vez de "image/jpeg"
    _types["jpeg"] = ".jpg";
    _types["png"]  = ".png";
    _types["gif"]  = ".gif";
    _types["webp"] = ".webp";
    _types["svg+xml"] = ".svg";
    _types["bmp"]  = ".bmp";
    _types["tiff"] = ".tiff";

    // --- AUDIO ---
    _types["audio/mpeg"] = ".mp3";
    _types["audio/wav"]  = ".wav";
    _types["audio/aac"]  = ".aac";
    
    // Casos curtos (se necessário)
    _types["mpeg"] = ".mp3";
    _types["wav"]  = ".wav";
    _types["aac"]  = ".aac";

    // --- VIDEO ---
    _types["video/mp4"]        = ".mp4";
    _types["video/x-matroska"] = ".mkv";
    _types["video/x-msvideo"]  = ".avi";
    
    // Casos curtos
    _types["mp4"]        = ".mp4";
    _types["x-matroska"] = ".mkv";
    _types["x-msvideo"]  = ".avi";

    // --- DOCUMENTOS / TEXTO ---
    _types["application/pdf"] = ".pdf";
    _types["text/plain"]      = ".txt";
    _types["text/csv"]        = ".csv";
    _types["application/json"] = ".json";
    
    // Microsoft Office (Nomes longos)
    _types["application/msword"] = ".doc";
    _types["application/vnd.openxmlformats-officedocument.wordprocessingml.document"] = ".docx";
    _types["application/vnd.ms-excel"] = ".xls";
    _types["application/vnd.openxmlformats-officedocument.spreadsheetml.sheet"] = ".xlsx";
    _types["application/vnd.ms-powerpoint"] = ".ppt";
    _types["application/vnd.openxmlformats-officedocument.presentationml.presentation"] = ".pptx";

    // Casos curtos de documentos
    _types["pdf"] = ".pdf";
    _types["plain"] = ".txt";
    _types["csv"] = ".csv";

    // binario
    _types["application/octet-stream"] = ".bin";
};

void RawProcessor::cleanup() {
    if (_outFile.is_open()) {
        _outFile.close();
    }

    if (!_fileName.empty()) {
        remove(_fileName.c_str());
        _fileName.clear();
    }
}

std::string RawProcessor::getExtension(const std::string& contentType) {

    if (_types.count(contentType)) {
        return _types[contentType];
    }
    return ".bin";
};

void RawProcessor::saveFile(const std::string& contentType, const std::string& uploadStore, const std::string& body) {

    std::string extension = getExtension(contentType);

    std::string uploadPath = uploadStore + "/upload_" + Utils::toString(std::time(0)) + extension;
    
    Utils::writeFile(uploadPath, body);
}

void RawProcessor::handleRawPost(const std::string& chunk) {
    
    if (_response == NULL)
        _response = new HttpResponse();
    if (_contentType.empty()) {
        _contentType = "application/octet-stream"; 
    }
    if (_types.count(_contentType)) {
        if (!append(chunk, chunk.size())) {
            cleanup();
            _response->setStatus(500);
            _response->setContentType("text/html");
            _response->setBody("Fail upload file");
            _isFinished = true;
        }
    } else {
        cleanup();
        _response->setStatus(415);
        _response->setContentType("text/html");
        _response->setBody("415 Unsupported Media Type: The server only supports form uploads.");
        _isFinished = true;
    }
};