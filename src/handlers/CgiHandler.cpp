#include "../../include/handlers/CgiHandler.hpp"
#include "../../include/bodyProcessor/BodyProcessorFactory.hpp"
#include "../../include/utils/Utils.hpp"

#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>

CgiHandler::CgiHandler(const ServerConfig& config, const HttpRequest& request, const Location& location)
: _config(config), _request(request), _location(location), _response(NULL), _isFinish(false),
  _body(""), _bodyProcessor(NULL)
{
}

CgiHandler::CgiHandler(const CgiHandler& other)
: _config(other._config), _request(other._request), _location(other._location),
  _response(NULL), _isFinish(other._isFinish), _body(other._body), _bodyProcessor(NULL)
{
    if (other._response != NULL)
        _response = new HttpResponse(*other._response);

    // não clona processor (não vale o risco)
}

CgiHandler::~CgiHandler() {
    if (_bodyProcessor != NULL) {
        delete _bodyProcessor;
        _bodyProcessor = NULL;
    }
    if (_response != NULL) {
        delete _response;
        _response = NULL;
    }
}

IMethodHandler* CgiHandler::clone() const {
    return new CgiHandler(*this);
}

/*
    ✅ REGRA:
    - Nunca execute CGI baseado em Content-Length dentro de chunked.
    - Use BodyProcessor:
        - chunked -> ChunkedBodyProcessor (deschunk + temp file)
        - normal  -> RawProcessor / etc
    - Quando processor termina, aí sim executa CGI.
*/
void CgiHandler::handleData(const std::string& chunk) {

    // GET / DELETE sem body: pode finalizar imediatamente na primeira chamada
    // (ou se o servidor chamar handleData("") para disparar)
    if (_request.getMethod() != "POST") {
        if (!_isFinish) {
            HttpResponse& r = process();
            (void)r;
            _isFinish = true;
        }
        return;
    }

    // POST
    if (_bodyProcessor == NULL) {
        _bodyProcessor = BodyProcessorFactory::createBodyProcessor(_config, _location, _request);
    }

    // Alimenta processor com o chunk (chunked ou não)
    _bodyProcessor->handleChunk(chunk);

    if (_bodyProcessor->isFinished()) {
        // Se for chunked, ChunkedBodyProcessor salvou caminho em request.bodyTempPath
        // Se não for chunked, pode estar em _request.getBody() dependendo do seu pipeline;
        // aqui vamos garantir um corpo final para o CGI:
        if (!_request.bodyTempPath.empty()) {
            // Melhor: não carregar tudo em RAM; vamos escrever direto no pipe depois
            // então não precisamos copiar para _body agora
        } else {
            // fallback: se seu fluxo já acumula em memória, garanta _body aqui
            _body = _request.getBody();
        }

        HttpResponse& r = process();
        (void)r;
        _isFinish = true;
    }
}

bool CgiHandler::isFinished() {
    return _isFinish;
}

std::string CgiHandler::getExtensionCgi() {

    std::string rawPath = _request.getPath();
    size_t q = rawPath.find('?');
    std::string path = (q != std::string::npos) ? rawPath.substr(0, q) : rawPath;

    size_t dot = path.find_last_of('.');
    if (dot == std::string::npos)
        return "";

    return path.substr(dot);
}

HttpResponse& CgiHandler::getResponse() {

    if (_response != NULL)
        return *_response;

    // Se chegou aqui sem ter executado no handleData, executa agora
    // (isso cobre GET sem body e alguns fluxos alternativos)
    return process();
}

size_t CgiHandler::getBodySizeFromTempFile(const std::string& path) const {
    struct stat st;
    if (stat(path.c_str(), &st) != 0)
        return 0;
    return (size_t)st.st_size;
}

std::string CgiHandler::readTempFileToString(const std::string& path) const {
    std::ifstream in(path.c_str(), std::ios::binary);
    if (!in.is_open())
        return "";
    std::string data;
    in.seekg(0, std::ios::end);
    data.resize((size_t)in.tellg());
    in.seekg(0, std::ios::beg);
    in.read(&data[0], data.size());
    return data;
}

std::vector<std::string> CgiHandler::buildCgiEnv(const HttpRequest& request, const Location& location,
                                                 const std::string& scriptPath, size_t contentLen)
{
    (void)location;
    std::vector<std::string> env;

    env.push_back("REQUEST_METHOD=" + request.getMethod());
    env.push_back("SCRIPT_FILENAME=" + scriptPath);
    env.push_back("QUERY_STRING=" + request.getQueryString());
    env.push_back("CONTENT_LENGTH=" + Utils::toString(contentLen));
    env.push_back("CONTENT_TYPE=" + request.getHeader("Content-Type"));
    env.push_back("GATEWAY_INTERFACE=CGI/1.1");
    env.push_back("SERVER_PROTOCOL=" + request.getHttpVersion());
    env.push_back("SERVER_SOFTWARE=WebServ/1.0");
    env.push_back("SERVER_NAME=localhost");
    env.push_back("SERVER_PORT=8080");
    env.push_back("PATH_INFO=" + request.getPath());
    env.push_back("REDIRECT_STATUS=200");

    return env;
}

void CgiHandler::spawnCgiChild(const HttpRequest& request, const Location& location,
                               const std::string& path, const std::string& interpreter,
                               int inPipe[2], int outPipe[2], size_t contentLen)
{
    dup2(inPipe[0], STDIN_FILENO);
    dup2(outPipe[1], STDOUT_FILENO);

    close(inPipe[1]);
    close(outPipe[0]);

    std::vector<std::string> env = buildCgiEnv(request, location, path, contentLen);
    char** envp = Utils::vecToCharArray(env);

    char* argv[] = {
        const_cast<char*>(interpreter.c_str()),
        const_cast<char*>(path.c_str()),
        NULL
    };

    execve(interpreter.c_str(), argv, envp);

    std::cerr << "CGI execve failed: " << strerror(errno) << std::endl;
    Utils::freeCharArray(envp);
    exit(EXIT_FAILURE);
}

std::string CgiHandler::readCgiOutput(int outPipe[2], pid_t pid)
{
    close(outPipe[1]);

    std::string output;
    char buffer[4096];
    ssize_t bytes;

    while ((bytes = read(outPipe[0], buffer, sizeof(buffer))) > 0)
        output.append(buffer, bytes);

    close(outPipe[0]);
    waitpid(pid, NULL, 0);

    return output;
}

HttpResponse& CgiHandler::responseHTTP(const std::string& output)
{
    // limpa response anterior, evita leak
    if (_response != NULL) {
        delete _response;
        _response = NULL;
    }
    _response = new HttpResponse();

    size_t headerEnd = output.find("\r\n\r\n");
    size_t sepLen = 4;

    if (headerEnd == std::string::npos) {
        headerEnd = output.find("\n\n");
        sepLen = 2;
    }

    if (headerEnd == std::string::npos) {
        _response->setStatus(500);
        _response->setContentType("text/html");
        _response->setBody("<h1>CGI Internal Error</h1>");
        _response->setConnectionClose(true);
        return *_response;
    }

    std::string headers = output.substr(0, headerEnd);
    std::string body    = output.substr(headerEnd + sepLen);

    int status = 200;

    // Status:
    size_t posStatus = headers.find("Status:");
    if (posStatus != std::string::npos) {
        size_t start = posStatus + 7;
        size_t end = headers.find("\n", start);
        std::string statusLine = Utils::trim(headers.substr(start, end - start));
        int code = std::atoi(statusLine.c_str());
        if (code >= 100 && code <= 599)
            status = code;
    }

    // Content-Type:
    std::string contentType = "text/html";
    size_t posCT = headers.find("Content-Type:");
    if (posCT != std::string::npos) {
        size_t start = posCT + 13;
        size_t end = headers.find("\n", start);
        contentType = Utils::trim(headers.substr(start, end - start));
    }

    // Location:
    size_t posLoc = headers.find("Location:");
    bool hasLocation = false;
    if (posLoc != std::string::npos) {
        size_t start = posLoc + 9;
        size_t end = headers.find("\r\n", start);
        std::string loc = Utils::trim(headers.substr(start, end - start));
        _response->setHeader("Location", loc);
        hasLocation = true;
    }

    _response->setStatus(status);
    _response->setContentType(contentType);

    if (status >= 300 && status < 400 && hasLocation) {
        _response->setBody("");
        _response->setHeader("Content-Length", "0");
        _response->setConnectionClose(true);
        return *_response;
    }

    _response->setBody(body);
    _response->setConnectionClose(true);
    return *_response;
}

HttpResponse& CgiHandler::process() {

    std::string interpreter;
    std::string extension = getExtensionCgi();

    std::string path = Utils::buildPathRequisition(_location.getPath(), _location.getRoot(), _request.getPath());
    std::string requestExt = Utils::getExtension(_request.getPath());

    if (_config.hasGlobalCGI && _config.hasExtGlobalCgi(requestExt))
        interpreter = _config.extAndPath.find(requestExt)->second;
    else
        interpreter = _location.getCgiPathForExtension(extension);

    int inPipe[2];
    int outPipe[2];
    if (pipe(inPipe) == -1 || pipe(outPipe) == -1) {
        if (_response != NULL) delete _response;
        _response = new HttpResponse();
        _response->setStatus(500);
        _response->setBody("<h1>500 Internal Server Error</h1>");
        _response->setConnectionClose(true);
        return *_response;
    }

    // Decide tamanho do body que será enviado ao CGI
    size_t contentLen = 0;
    bool useTemp = !_request.bodyTempPath.empty();

    if (_request.getMethod() == "POST") {
        if (useTemp)
            contentLen = getBodySizeFromTempFile(_request.bodyTempPath);
        else
            contentLen = _body.size();
    }

    pid_t pid = fork();
    if (pid < 0) {
        if (_response != NULL) delete _response;
        _response = new HttpResponse();
        _response->setStatus(500);
        _response->setBody("<h1>500 Internal Server Error</h1>");
        _response->setConnectionClose(true);
        return *_response;
    }

    if (pid == 0) {
        spawnCgiChild(_request, _location, path, interpreter, inPipe, outPipe, contentLen);
    }

    // PAI
    close(inPipe[0]);
    close(outPipe[1]);

    // Escreve body pro CGI e fecha stdin (EOF)
    if (_request.getMethod() == "POST" && contentLen > 0) {

        if (useTemp) {
            std::ifstream in(_request.bodyTempPath.c_str(), std::ios::binary);
            if (in.is_open()) {
                char buf[4096];
                while (in.good()) {
                    in.read(buf, sizeof(buf));
                    std::streamsize n = in.gcount();
                    if (n > 0) {
                        ssize_t w = write(inPipe[1], buf, (size_t)n);
                        if (w < 0) break;
                    }
                }
            }
        } else {
            ssize_t w = write(inPipe[1], _body.c_str(), _body.size());
            (void)w;
        }
    }

    close(inPipe[1]); // ✅ EOF -> CGI entende “fim do body”

    std::string output = readCgiOutput(outPipe, pid);
    return responseHTTP(output);
}
