#include "../../include/core/WebServer.hpp"
#include "../../include/core/Server.hpp"
#include "../../include/http/HttpRequest.hpp"
#include "../../include/http/HttpResponse.hpp"
#include "../../include/handlers/RequestHandler.hpp"
#include "../../include/config/Config.hpp"
#include "../../include/utils/Utils.hpp"
#include "../../include/utils/ErrorPage.hpp"

Server::Server(const Config &config) : _config(config), _running(true), _CLIENT_TIMEOUT(CLIENT_TIMEOUT) {}

Server::~Server() {
    for (size_t i = 0; i < server_fds.size(); i++) {
        epoll_ctl(this->epoll_fd, EPOLL_CTL_DEL, server_fds[i], NULL);
        close(server_fds[i]);
    }

    for (std::map<int, Client>::iterator it = clients.begin(); it != clients.end(); ++it) {
        epoll_ctl(this->epoll_fd, EPOLL_CTL_DEL, it->first, NULL);
        close(it->first);
    }

    for (std::map<int, CgiProcess *>::iterator it = _cgiByFd.begin(); it != _cgiByFd.end(); ++it) {

        CgiProcess* cgi = it->second;
        if (!cgi->stdin_closed) {
            epoll_ctl(this->epoll_fd, EPOLL_CTL_DEL, cgi->stdin_fd, NULL);
            close(cgi->stdin_fd);
        }
        if (!cgi->stdout_closed) {
            epoll_ctl(this->epoll_fd, EPOLL_CTL_DEL, cgi->stdout_fd, NULL);
            close(cgi->stdout_fd);
        }
        delete cgi;
    }
    close(epoll_fd);
}

void Server::initAllSockets() {
    for (size_t i = 0; i < _config.servers.size(); i++) {
        ServerConfig sc = _config.servers[i];
        int fd = sc.initSocket();
        server_fds.push_back(fd);
        server_by_fd[fd] = sc;
    }
}

void Server::registerSocketsInEpoll() {
    epoll_fd = epoll_create(1);
    if (epoll_fd == -1) {
        perror("epoll_create");
        exit(1);
    }

    for (size_t i = 0; i < server_fds.size(); i++) {
        epoll_event ev;
        ev.events = EPOLLIN;
        ev.data.fd = server_fds[i];
        epoll_ctl(epoll_fd, EPOLL_CTL_ADD, server_fds[i], &ev);
    }
}

void Server::handleNewConnection(int server_fd) {
    int client_fd = accept(server_fd, NULL, NULL);
    if (client_fd < 0) return;
    fcntl(client_fd, F_SETFL, O_NONBLOCK);

    Client client(client_fd);
    client_server[client_fd] = &server_by_fd[server_fd];
    clients[client_fd] = client;
    
    epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_fd, &clients[client_fd].getDataEvent());
    std::cout << GREEN << "New client in the server " << server_by_fd[server_fd].getPort() << RESET << std::endl;
    logClienteConected(client_fd);
    updateClientActivity(client_fd);
}

const Location &Server::findLocation(ServerConfig *serverCfg, HttpRequest &request) {
    const std::map<std::string, Location> &locs = serverCfg->getLocations();
    std::string path = request.getPath();
    std::string match = "/";
    size_t longestMatch = 0;

    for (std::map<std::string, Location>::const_iterator it = locs.begin(); it != locs.end(); ++it) {
        const std::string &locPath = it->first;

        if (path.compare(0, locPath.size(), locPath) != 0)
            continue;

        if (path.size() > locPath.size() && locPath != "/" && path[locPath.size()] != '/')
            continue;

        if (locPath.size() > longestMatch) {
            longestMatch = locPath.size();
            match = locPath;
        }
    }

    return locs.at(match);
}

void Server::removeClient(int client_fd) {
    epoll_ctl(this->epoll_fd, EPOLL_CTL_DEL, client_fd, NULL);
    removeMethodHandler(clients[client_fd]);
    clients.erase(client_fd);
    close(client_fd);
}

void Server::readClientBuffer(const int& client_fd, char* buffer, size_t bufSize, int& bytes) {

    // O epoll garantiu (EPOLLIN) que há dados ou um evento de fechamento. Então se retornar -1 e provavelmente um erro.
    bytes = recv(client_fd, buffer, bufSize, 0);

    // Sucesso, vamos tratar o buffer continuando com o fluxo atual
    if (bytes > 0) {
        return ;
    } else if (bytes == 0) {
        removeClient(client_fd);
        logClientDesconected(client_fd);
    }

    // Se bytes e == 0 o cliente notificou que vai se desconectar
    // Se valor < 0 provavelmente e um erro pois o epoll nos garantiu que havia dados para ler
    if (bytes == 0) {
        logClientDesconected(client_fd);
    } else {
        std::cerr << "[FD " << client_fd << "] Error reading socket, closing connection" << std::endl;
    }
    removeClient(client_fd);
}

IMethodHandler* Server::buildMethodHandler(Client& client , int &client_fd) {

    HttpRequest& request = client.getRequest();
    const Location &location = findLocation(client_server[client_fd], request);
    const ServerConfig& serverConfig = *client_server[client_fd];
    return RequestHandler::handle(serverConfig, request, location, client_fd);
}

bool Server::removeMethodHandler(Client& client) {
    
    if (client.handler) {
        delete client.handler;
        client.handler = NULL;
    }

    return true;
}

void Server::finalizeClientConnection(const int &client_fd, Client& client, const bool& closeConnection) {

    if (closeConnection) {
        removeClient(client_fd);
    } else {
        client.cleanData();
    }
}

void Server::logClienteRequest(const int &client_fd, Client& client) {

    std::cout << "[FD " << client_fd << "] <--- Request: " 
            << client.getRequest().getMethod() << " " 
            << client.getRequest().getPath() << " "
            << client.getRequest().getHttpVersion()
            << std::endl;
}

void Server::logClienteConected(const int &client_fd) {
    std::cout << "[FD " << client_fd << "] (+) Connection established" << std::endl;
}

void Server::logClientDesconected(const int &client_fd) {
    std::cout << RED << "[FD " << client_fd << "] (-) Connection closed" << RESET << std::endl;
}

void Server::logStatusResponse(const int &client_fd, Client& client) {
    std::cout << "[FD " << client_fd << "]" << " ---> " << client.getRequest().getMethod() << " " << client.getRequest().getPath() << " (" << client.getCodeResponseStatus() <<  ") - "<< client.getLenBody() << " bytes" << std::endl;
}

void Server::prepareResponse(const int &client_fd, Client& client) {
    
    HttpResponse& resp = client.handler->getResponse();
    resp.setConnectionClose(client.getCloseConnection());
    client.setResponse(resp.toString());
    // client.setCloseConnection(resp.isConnectionClose());
    client.setCodeResponseStatus(resp.getStatusResponse());

    epoll_event& event = client.getDataEvent();
    event.events = EPOLLOUT;
    epoll_ctl(this->epoll_fd, EPOLL_CTL_MOD, client_fd, &event);
    // finalizeClientConnection(client_fd, client, closeConnection);
}

void Server::addBuffer(Client& client, char* buffer, int& bytes) {
    // Enquanto os headers ainda NÃO foram totalmente recebidos
    if (!client.isAllHeaders()) {
        // Acumula dados no buffer interno do Client
        // Aqui ainda estamos lendo APENAS headers HTTP
        client.addBuffer(std::string(buffer, bytes));

        // Se após adicionar esse bloco os headers ficaram completos
        if (client.isAllHeaders()) {
            // Loga a requisição assim que headers completos existem
            logClienteRequest(client.getClienteFd(), client);

            // Recupera o body que veio junto com os headers
            std::string remainingBody = client.extractBodyAfterHeaders();

            // Verifica se o framing do corpo é chunked
            if (client.getRequest().isChunked()) {

                // Marca o Client como chunked
                // Chunked é ESTADO, não uma decisão pontual
                client.setChunked(true);

                // Inicializa o decoder de chunked
                // Esse decoder será responsável por:
                //  - Ler tamanhos hexadecimais
                //  - Concatenar os chunks
                //  - Detectar o chunk final (0\r\n\r\n)
                client.initChunkedDecoder(); // se existir

                // alimenta o decoder com o resto
                if (!remainingBody.empty()) {
                    client.feedChunked(
                        remainingBody.c_str(),
                        remainingBody.size()
                    );
                }
            }
        }
    }
    else {
        // Headers já foram lidos → agora estamos lidando com o BODY

        // Se a requisição usa Transfer-Encoding: chunked
        if (client.isChunked() && !client.isChunkedFinished()) {

            // Alimenta o decoder de chunked com dados do socket
            // O decoder decide quando o corpo está completo
            // O handler NÃO deve ver chunk framing ex: 4\r\nWiki\r\n5\r\npedia\r\n0\r\n\r\n
            client.feedChunked(buffer, bytes);
        } else {
            // Corpo normal (Content-Length)
            // Aqui o body é acumulado diretamente
            client.addBody(std::string(buffer, bytes));
        }
    }
}  

void Server::epoll_add(int fd, uint32_t events) {
    struct epoll_event ev;
    std::memset(&ev, 0, sizeof(ev));

    ev.events  = events;
    ev.data.fd = fd;

    epoll_ctl(this->epoll_fd, EPOLL_CTL_ADD, fd, &ev);
}

void Server::handleClientRequest(int client_fd) {
    
    char buffer[8192];
    int bytes = 0;

    readClientBuffer(client_fd, buffer, sizeof(buffer), bytes);
    if (bytes <= 0) {
        return ;
    }
    Client& client = clients[client_fd];
    addBuffer(client, buffer, bytes);
    if (!client.isAllHeaders())
        return ;
    if (client.handler == NULL)
        client.handler = buildMethodHandler(client, client_fd);
    if (client.isChunked()){
        // std::cout << client.getRequest().getBody() << std::endl;
        if(!client.isChunkedFinished())
            return;

        if (!client.isBodyDelivered()) {
            // std::cout << "Vou setar o body do chuncked, tamanho atual: " << client.getChunkedBody().size() << std::endl;
            client.getRequest().setBody(client.getChunkedBody());
            client.handler->handleData(client.getRequest().getBody());
            client.eraseBody();
            client.setBodyDelivered(true);
        }
    }
    else {
        client.handler->handleData(client.getRequest().getBody());
        client.eraseBody();
    }
    if (client.handler->isFinished()) {
        if (client.handler->isCgi() && !client.handler->isError()) {
            startCgiForClient(client);
        }
        else
            prepareResponse(client_fd, client);
    }
    updateClientActivity(client_fd);
}

void Server::sendResponseClient(int client_fd) {

    // epoll nos garantiu o EPOLLOUT, então o socket esta disponivel para escrita
    Client& client = clients[client_fd];
    const size_t MAX_SEND = 8192; // ou 4096
    size_t remaining = client.getLenBody() - client.getBytesSend();
    size_t toSend = std::min(MAX_SEND, remaining);
    // std::cout << "Tamanho do corpo a ser enviado: " << client.getLenBody() << " Enviado: " <<  client.getBytesSend() << std::endl;
    ssize_t bytesSend = send(client_fd, (client.getResponse().data() + client.getBytesSend()),  toSend, 0);

    // Falha ao escrever no socket do cliente mesmo epoll nos garantindo que podiamos escrever
    if (bytesSend < 0) {
        std::cout << "[FD " << client_fd << "] Error while send the response" << std::endl;
        removeClient(client_fd);
        return;
    }

    // Adicionamos a quantidade de bytes que foram enviados, nem sempre vai ser total então pegamos o retorno do send que diz quantos bytes foram realmente enviados; Também e possivel que seja enviado 0 bytes
    client.addBytesSend(bytesSend);
    if ((size_t)client.getBytesSend() == client.getLenBody()) {

        logStatusResponse(client_fd, client);
        epoll_event& event = client.getDataEvent();
        event.events = EPOLLIN;
        epoll_ctl(this->epoll_fd, EPOLL_CTL_MOD, client_fd, &event);
        removeMethodHandler(client);
        finalizeClientConnection(client_fd, client, client.getCloseConnection());
    }
    updateClientActivity(client_fd);
};

void Server::handleCgiWrite(int fd) {

    // Quando entra aqui o epoll nos garantiu que era possivel escrever no pipe
    CgiProcess* cgi = _cgiByFd[fd];

    // std::cout << "Escrevendo no pipe" << std::endl;
    const size_t MAX_WRITE = 4096; // PIPE_BUF seguro

    size_t remaining = cgi->input.size() - cgi->input_offset;

    size_t toWrite = std::min(MAX_WRITE, remaining);


    ssize_t n = write(fd, (cgi->input.data() + cgi->input_offset), toWrite);
    if (n > 0) {
        cgi->input_offset += n;

        if (cgi->input_offset == cgi->input.size()) {
            if (!cgi->stdin_closed) {
                // removemos da lista de interesse do epoll
                epoll_ctl(this->epoll_fd, EPOLL_CTL_DEL, fd, 0);
                close(fd);
                cgi->stdin_closed = true;
                _cgiByFd.erase(cgi->stdin_fd);
            }
        }
    }
    // erro ao tentar escrever no stdin do pipe no processo, já que o epoll nos mandou EPOLLOUT
    // Necessario encerrar o processo e jogar o erro para o cliente ?
    else {
        std::cerr << "Error writing to CGI stdin. Closing pipe." << std::endl;
        epoll_ctl(this->epoll_fd, EPOLL_CTL_DEL, fd, 0);
        cgi->stdin_closed = true;
        _cgiByFd.erase(fd);
        close(fd);
    }
    updateCgiActivity(fd);
}

void Server::startCgiForClient(Client& client) {
    
    CgiHandler* cgiHandler = static_cast<CgiHandler*>(client.handler);

    CgiProcess* cgi = cgiHandler->startCgi();
    cgi->last_activity = time(NULL);

    int errorCode = 0;
    switch (cgi->status)
    {
        case CgiProcess::CGI_FORBIDDEN:
            errorCode = 403;
            break;
        case CgiProcess::CGI_NOT_FOUND:
            errorCode = 404;
            break;
        case CgiProcess::CGI_INTERNAL_ERROR:
            errorCode = 500;
            break;
        case CgiProcess::CGI_OK:
            errorCode = 0;
            break;
    }

    if (errorCode != 0) {

        HttpResponse response;

        response.setStatus(errorCode);
        response.setConnectionClose(true);
        response.setBody(ErrorPage::build(errorCode));
        response.setContentType("text/html");
        client.setCloseConnection(response.isConnectionClose());
        client.setCodeResponseStatus(response.getStatusResponse());
        client.setResponse(response.toString());
        delete cgi;
        epoll_event& event = client.getDataEvent();
        event.events = EPOLLOUT;
        // Modificar o cliente para poder escrever no socket
        epoll_ctl(this->epoll_fd, EPOLL_CTL_MOD, client.getClienteFd(), &event);
        return ;
    }
        
    if (_cgiByFd.find(cgi->stdin_fd) != _cgiByFd.end() || _cgiByFd.find(cgi->stdout_fd) != _cgiByFd.end()) {
        std::cout << "Pipe já registrado =================================" << std::endl;
    }
    _cgiByFd[cgi->stdin_fd]  = cgi;
    _cgiByFd[cgi->stdout_fd] = cgi;
        
    epoll_add(cgi->stdout_fd, EPOLLIN);
        
    if (!cgi->input.empty()) {
        epoll_add(cgi->stdin_fd, EPOLLOUT);
    }
    else {
        close(cgi->stdin_fd);
        cgi->stdin_closed = true;
        _cgiByFd.erase(cgi->stdin_fd);
    }
}

void Server::removeCgiFd(const int& fd) {

    epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fd, 0);
    close(fd);
    _cgiByFd.erase(fd);
}

void Server::finalizeCgiResponse(CgiProcess* cgi) {
    
    if (clients.find(cgi->client_fd) == clients.end()) {
        std::cerr << "Erro: CGI finalizou, mas o cliente (fd " << cgi->client_fd << ") já desconectou." << std::endl;
        if (!cgi->stdin_closed) {
            cgi->stdin_closed = true;
            removeCgiFd(cgi->stdin_fd);
        }
        if (!cgi->stdout_closed) {
            cgi->stdin_closed = true;
            removeCgiFd(cgi->stdout_fd);
        }
        delete cgi;
        return;
    }

    Client& client = clients[cgi->client_fd];

    if (client.handler == NULL) {
        std::cerr << "Erro: Cliente existe, mas o handler é NULL." << std::endl;
        if (!cgi->stdin_closed) {
            cgi->stdin_closed = true;
            removeCgiFd(cgi->stdin_fd);
        }
        if (!cgi->stdout_closed) {
            cgi->stdout_closed = true;
            removeCgiFd(cgi->stdout_fd);
        }
        delete cgi;
        return;
    }

    HttpResponse response;

    CgiHandler* cgiHandler = static_cast<CgiHandler*>(client.handler);
    
    response = cgiHandler->responseHTTP(cgi->output);
    
    
    client.setCloseConnection(response.isConnectionClose());
    client.setCodeResponseStatus(response.getStatusResponse());
    
    client.setResponse(response.toString());

    // Cleanup
    if (!cgi->stdin_closed) {
        cgi->stdin_closed = true;
        removeCgiFd(cgi->stdin_fd);
    }
    if (!cgi->stdout_closed) {
        cgi->stdout_closed = true;
        removeCgiFd(cgi->stdout_fd);
    }

    epoll_event& event = client.getDataEvent();
    event.events = EPOLLOUT;
    // Modificar o cliente para poder escrever no socket
    epoll_ctl(this->epoll_fd, EPOLL_CTL_MOD, client.getClienteFd(), &event);

    delete cgi;
}

void Server::handleCgiRead(int fd) {

    // Epoll nos notificou que tinha dados para ler
    CgiProcess* cgi = _cgiByFd[fd];
    char buffer[4096];


    ssize_t n = read(fd, buffer, sizeof(buffer));
    // Vamos ler o que o pipe tinha e armazenar o resultado
    if (n > 0) {
        cgi->output.append(buffer, n);
        updateCgiActivity(fd);
    }
    else {
        // Se n == 0 então chegamos no EOF, e esta tudo pronto para enviarmos para o cliente
        if (n == 0) {
            if (!cgi->stdin_closed) {
                cgi->stdin_closed = true;
                removeCgiFd(cgi->stdin_fd);
            }
            if (!cgi->stdout_closed) {
                cgi->stdout_closed = true;
                removeCgiFd(cgi->stdout_fd);
            }
            waitpid(cgi->pid, NULL, 0);
            finalizeCgiResponse(cgi);
        }
        // Se o n < 0 então e provavelmente um erro pois o epoll nos notificou que tinha dados para leitura
        else {
            
            std::cerr << "[ " << cgi->client_fd << " ]" << " Error reading from CGI stdout (FD " << fd << "). Trusting epoll to close." << std::endl;
            if (!cgi->stdin_closed) {
                cgi->stdin_closed = true;
                removeCgiFd(cgi->stdin_fd);
            }
            if (!cgi->stdout_closed) {
                cgi->stdout_closed = true;
                removeCgiFd(cgi->stdout_fd);
            }
            //remover cliente também ?
            delete cgi;
        }
        updateCgiActivity(fd);
    }
}

void Server::handleCgiEvent(int fd, uint32_t ev) {
    CgiProcess* cgi = _cgiByFd[fd];

    if ((ev & EPOLLOUT) && fd == cgi->stdin_fd) {
        handleCgiWrite(fd);
    }
    
    if ((ev & (EPOLLIN | EPOLLHUP)) && fd == cgi->stdout_fd) {
        handleCgiRead(fd);
    }
    updateClientActivity(fd);
    updateCgiActivity(fd);
}

void Server::eventLoop() {
    while (_running) {
        int n = epoll_wait(epoll_fd, events, 64, 1000);

        for (int i = 0; i < n; i++) {
            int fd = events[i].data.fd;

          if (server_by_fd.count(fd)) {
                handleNewConnection(fd);
            } else if (_cgiByFd.count(fd)) {
                handleCgiEvent(fd, events[i].events);
            } else if (events[i].events & EPOLLERR) {
                removeClient(fd);
                continue;
            }
            else {
                if (events[i].events & EPOLLIN) {
                    handleClientRequest(fd);
                }
                if (events[i].events & EPOLLOUT) {
                    sendResponseClient(fd);
                }
            }
        }
        checkClientTimeouts();
        checkCgiTimeouts();
    }
}

void Server::shutdown() {
    _running = false;
}

void signalHandler(int signum) {
    (void)signum;
    waitpid(-1, NULL, WNOHANG);
}

void Server::updateClientActivity(int fd) {
    clients[fd].setLastActivity(time(NULL));
}

void Server::checkClientTimeouts() {
    time_t now = time(NULL);
    std::map<int, Client>::iterator it = clients.begin();

    while (it != clients.end()) {
        std::map<int, Client>::iterator current = it++;

        if (now - current->second.getLastActivity() > _CLIENT_TIMEOUT) {
            int fd = current->first;
            std::cout << "[FD " << fd  << "] " << RED << "Connection timeout" << RESET << std::endl;
            removeClient(fd);
        }
    }
}

void Server::updateCgiActivity(int fd) {
    if (_cgiByFd.count(fd))
        _cgiByFd[fd]->last_activity = time(NULL);
}

void Server::checkCgiTimeouts() {
    time_t now = time(NULL);
    
    for (std::map<int, CgiProcess*>::iterator it = _cgiByFd.begin(); it != _cgiByFd.end(); ) {
        CgiProcess* cgi = it->second;
        std::map<int, CgiProcess*>::iterator current = it++;
        
        if (now - cgi->last_activity > CGI_TIMEOUT) {
            int fd = current->first;
            std::cout << RED << "[CGI FD " << fd << "] Timeout after " 
                      << CGI_TIMEOUT << "s. Killing process PID " << cgi->pid << RESET << std::endl;
            
            killTimedOutCgi(cgi);
        }
    }
}

void Server::killTimedOutCgi(CgiProcess* cgi) {
    if (!cgi->stdin_closed) {
        epoll_ctl(epoll_fd, EPOLL_CTL_DEL, cgi->stdin_fd, NULL);
        close(cgi->stdin_fd);
        cgi->stdin_closed = true;
    }

    if (!cgi->stdout_closed) {
        epoll_ctl(epoll_fd, EPOLL_CTL_DEL, cgi->stdout_fd, NULL);
        close(cgi->stdout_fd);
        cgi->stdout_closed = true;
    }
    
    if (cgi->pid > 0) {
        kill(cgi->pid, SIGTERM);
        int status;
        waitpid(cgi->pid, &status, WNOHANG);
    }
    
    _cgiByFd.erase(cgi->stdin_fd);
    _cgiByFd.erase(cgi->stdout_fd);
    
    if (clients.find(cgi->client_fd) != clients.end()) {
        Client& client = clients[cgi->client_fd];
        HttpResponse resp;
        resp.setStatus(504);
        resp.setConnectionClose(true);
        resp.setBody(ErrorPage::build(504));
        resp.setContentType("text/html");
        
        client.setResponse(resp.toString());
        client.setCodeResponseStatus(resp.getStatusResponse());
        client.setCloseConnection(true);
        
        epoll_event& event = client.getDataEvent();
        event.events = EPOLLOUT;
        epoll_ctl(epoll_fd, EPOLL_CTL_MOD, cgi->client_fd, &event);
        removeMethodHandler(client);
    }
    
    delete cgi;
}

void Server::start() {
    signal(SIGCHLD, signalHandler);
    signal(SIGPIPE, SIG_IGN);
    initAllSockets();
    registerSocketsInEpoll();
    eventLoop();
}
