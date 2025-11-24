#include "../../include/handlers/PostHandler.hpp"
#include "../../include/utils/Utils.hpp"

std::string PostHandler::getImageExtension(const std::string& typeImage) {
    
    if(typeImage.compare("jpeg") == 0)
        return (".jpg");
    else if(typeImage.compare("png") == 0)
        return (".png");
    else if(typeImage.compare("gif") == 0)
        return (".gif");
    else if(typeImage.compare("webp") == 0)
        return (".webp");
    else if(typeImage.compare("svg+xml") == 0)
        return (".svg");
    else if(typeImage.compare("bmp") == 0)
        return (".bmp");
    else if(typeImage.compare("tiff") == 0)
        return (".tiff");
    return "";
};

std::string PostHandler::getAudioExtension(const std::string& typeAudio) {
    
    if(typeAudio.compare("mpeg") == 0)
        return (".mp3");
    else if(typeAudio.compare("wav") == 0)
        return (".wav");
    else if(typeAudio.compare("aac") == 0)
        return (".aac");
    return "";
};

std::string PostHandler::getVideoExtension(const std::string& typeVideo) {
    
    if(typeVideo.compare("mp4") == 0)
        return (".mp4");
    else if(typeVideo.compare("x-matroska") == 0)
        return (".mkv");
    else if(typeVideo.compare("x-msvideo") == 0)
        return (".avi");
    return "";
};

std::string PostHandler::getDocExtension(const std::string& typeDoc) {
    
    if(typeDoc.compare("pdf") == 0)
        return (".pdf");
    else if(typeDoc.compare("application/msword") == 0)
        return (".doc");
    else if(typeDoc.compare("vnd.openxmlformats-officedocument.wordprocessingml.document") == 0)
        return (".docx");
    else if(typeDoc.compare("vnd.ms-excel") == 0)
        return (".xls");
    else if(typeDoc.compare("vnd.openxmlformats-officedocument.spreadsheetml.sheet") == 0)
        return (".xlsx");
    else if(typeDoc.compare("vnd.ms-powerpoint") == 0)
        return (".ppt");
    else if(typeDoc.compare("vnd.openxmlformats-officedocument.presentationml.presentation") == 0)
        return (".pptx");
    else if(typeDoc.compare("plain") == 0)
        return (".txt");
    else if(typeDoc.compare("csv") == 0)
        return (".csv");
    return "";
};

void PostHandler::saveAsImage(const std::string& typeImage, const std::string& uploadStore, const std::string& body) {

    std::string uploadPath = uploadStore + "/upload_" + Utils::toString(std::time(0)) + getImageExtension(typeImage);
    Utils::writeFile(uploadPath, body);
}

void PostHandler::saveAsAudio(const std::string& typeAudio, const std::string& uploadStore, const std::string& body) {

    std::string uploadPath = uploadStore + "/upload_" + Utils::toString(std::time(0)) + getAudioExtension(typeAudio);
    Utils::writeFile(uploadPath, body);
}

void PostHandler::saveAsVideo(const std::string& typeVideo, const std::string& uploadStore, const std::string& body) {

    std::string uploadPath = uploadStore + "/upload_" + Utils::toString(std::time(0)) + getVideoExtension(typeVideo);
    Utils::writeFile(uploadPath, body);
}

void PostHandler::saveAsDoc(const std::string& typeDoc, const std::string& uploadStore, const std::string& body) {

    std::string uploadPath = uploadStore + "/upload_" + Utils::toString(std::time(0)) + getDocExtension(typeDoc);
    Utils::writeFile(uploadPath, body);
}

bool PostHandler::isDocFile(const std::string& contentType) {

    if (contentType.compare("application/pdf") == 0 || contentType.compare("application/msword") == 0
        || contentType.compare("application/vnd.openxmlformats-officedocument.wordprocessingml.document") == 0 || contentType.compare("application/vnd.ms-excel") == 0
        || contentType.compare("application/vnd.openxmlformats-officedocument.spreadsheetml.sheet") == 0 || contentType.compare("application/vnd.ms-powerpoint") == 0
        || contentType.compare("application/vnd.openxmlformats-officedocument.presentationml.presentation") == 0 || contentType.compare("text/plain") == 0
        || contentType.compare("text/csv") == 0) {
            return (true);
    }
    return (false);
}

HttpResponse PostHandler::process(HttpRequest &request, const Location &location)
{
    std::string contentType = request.getHeader("Content-Type");
    std::string body = request.getBody();
    HttpResponse response;
    response.setHttpVersion(request.getHttpVersion());


    // std::cout << request.getBuffer() << std::endl;
    

    if (contentType.compare(0, 6, "image/") == 0) {
        saveAsImage(contentType.substr(6, contentType.size()), location.getUploadStore(), body);
        response.setStatus(201);
        response.setContentType("text/html");
        response.setBody("File uploaded successfully to " + location.getUploadStore());
        return response;
    } else if (contentType.compare(0, 6, "audio/") == 0) {
        saveAsAudio(contentType.substr(6, contentType.size()), location.getUploadStore(), body);
        response.setStatus(201);
        response.setContentType("text/html");
        response.setBody("File uploaded successfully to " + location.getUploadStore());
        return response;
    } else if (contentType.compare(0, 6, "video/") == 0) {
        saveAsVideo(contentType.substr(6, contentType.size()), location.getUploadStore(), body);
        response.setStatus(201);
        response.setContentType("text/html");
        response.setBody("File uploaded successfully to " + location.getUploadStore());
        return response;
    } else if (isDocFile(contentType)) {

        size_t pos = contentType.find("/");
        if (pos != std::string::npos) {
            saveAsDoc(contentType.substr(pos + 1), location.getUploadStore(), body);
            response.setStatus(201);
            response.setContentType("text/html");
            response.setBody("File uploaded successfully to " + location.getUploadStore());
        } else {
            response.setStatus(415);
            response.setContentType("text/html");
            response.setBody("415 Unsupported Media Type: The server only supports form uploads.");
        }
        return response;
    }

    // diferenciar comportamento baseado no Content-Type
    if (contentType == "application/x-www-form-urlencoded") {
        // Processar dados de formulário simples
        std::string uploadPath = location.getUploadStore() + "/upload_" + Utils::toString(std::time(0)) + ".txt";
        std::cout << "Body (" << body.size() << " bytes): [" << body << "]" << std::endl;
        if (Utils::writeFile(uploadPath, body)) {
            std::cout << request.getPath() << std::endl;
            response.setStatus(201);
            response.setContentType("text/html");
            response.setBody("File uploaded successfully to " + uploadPath);
        } else {
            response.setStatus(500);
            response.setContentType("text/html");
            response.setBody("500 Internal Server Error: Failed to save the uploaded file.");
        }
    } else if (contentType.find("multipart/form-data") != std::string::npos) {
        // Processar upload de arquivo multipart
        // Aqui você precisaria implementar o parsing do corpo multipart
        // Para simplificação, vamos assumir que o arquivo foi salvo com sucesso
        std::string uploadPath = location.getUploadStore() + "/upload_" + Utils::toString(std::time(0)) + ".bin";
        if (Utils::writeFile(uploadPath, body)) {
            response.setStatus(201);
            response.setContentType("text/html");
            response.setBody("File uploaded successfully to " + uploadPath);
        } else {
            response.setStatus(500);
            response.setContentType("text/html");
            response.setBody("500 Internal Server Error: Failed to save the uploaded file.");
        }
    } else {
        // Tipo de conteúdo não suportado
        response.setStatus(415);
        response.setContentType("text/html");
        response.setBody("415 Unsupported Media Type: The server only supports form uploads.");
    }
    return response;
}

