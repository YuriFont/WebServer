#include "../../include/handlers/PostHandler.hpp"
#include "../../include/utils/Utils.hpp"

//O tamanho da requisição está no content-length (tamanho em bytes do tamanho da requisição)
//se for maior que o config - error e devolve pro usuario - verificar - client_max_body_size
//Erro do servidor ou do cliente? Do cliente
//Content too large 413

HttpResponse PostHandler::process(HttpRequest &request, const Location &location, const ServerConfig &serverConfig)
{
    std::string contentType = request.getHeader("Content-Type");
    std::string body = request.getBody();
    size_t bodySize = request.getBody().size();
    size_t maxSize = serverConfig.client_max_body_size;
    HttpResponse response;
    
    if (bodySize > maxSize){
        response.setStatus(413);
        response.setContentType("text/html");
        response.setBody("<h1>413 Request Entity Too Large</h1>");
        response.setConnectionClose(true);
        return response;
    }

    if (contentType == "image/jpeg") {
        std::string uploadPath = location.getUploadStore() + "/upload_" + Utils::toString(1) + ".jpg";
        Utils::writeFile(uploadPath, body);
        response.setStatus(201);
        response.setContentType("text/html");
        response.setBody("File uploaded successfully to " + uploadPath);
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

