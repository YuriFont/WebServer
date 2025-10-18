#include "../include/Server.hpp"
#include "../include/Utils.hpp"
#include "../include/Config.hpp"
#include "../include/HttpRequest.hpp"
#include "../include/HttpResponse.hpp"

#include <iostream>
#include <cstring>
#include <cstdlib>
#include <unistd.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/epoll.h>

#include <sys/stat.h>

Server::Server(const Config &config) : _config(config) {}

Server::~Server() {}

const Location &Server::findLocation(HttpRequest &request)
{

    std::string path = request.getPath();
    std::map<std::string, Location>::const_iterator it = this->_config.locations.begin();
    size_t longestMatch = 0;
    std::string match = "/";

    while (it != this->_config.locations.end())
    {

        if ((path.compare(0, it->first.size(), it->first) == 0) && (it->first.size() > longestMatch))
        {
            longestMatch = it->first.size();
            match = it->first;
        }
        ++it;
    }
    std::map<std::string, Location>::const_iterator found = this->_config.locations.find(match);
    if (found == this->_config.locations.end())
    {
        throw std::runtime_error("Nenhum location encontrado para path: " + path);
    }
    return found->second;
}

std::string Server::fileToString(const Location &location, const std::string &nameFile)
{

    // readonly esta ficando com www/readonly e ja tem o namefile como /readonly, se concatenar vai ficar com  www/readonly/readonly
    // std::cout << location.getRoot() << std::endl;
    // std::cout << nameFile << std::endl;

    // mudar para o index do location
    // if (nameFile == "/") {
    //     nameFile = "/index.html"
    // }

    std::string body = "";
    std::string filePath = location.getRoot() + (nameFile == "/" ? "/index.html" : nameFile);

    struct stat sb;

    if (stat(filePath.c_str(), &sb) == 0)
    {
        std::cout << "arquivo existe" << std::endl;
    }
    else
    {
        // retorna ai
        return "<h1>Not Found</h1>";
        std::cout << "arquivo não existe" << std::endl;
    }

    int fd = open(filePath.c_str(), O_RDONLY);
    if (fd != -1)
    {

        char buffer[1024];

        int bytes = read(fd, buffer, sizeof(buffer) - 1);
        while (bytes > 0)
        {

            buffer[bytes] = '\0';
            body += buffer;
            bytes = read(fd, buffer, sizeof(buffer) - 1);
        }
    }

    close(fd);

    return (body);
}

void Server::initSocket()
{

    this->server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (this->server_fd == -1)
    {
        strerror(errno);
        exit(EXIT_FAILURE);
    }

    // não bloqueante, serve para não ficar esperando quando tiver em uma conexão, F_SETFL serve para modificar o descritor de arquivos
    fcntl(this->server_fd, F_SETFL, O_NONBLOCK);

    struct sockaddr_in addr;
    int opt = 1;
    socklen_t addrlen = sizeof(addr);

    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(4242);

    // permitir reuso da porta sem o timeout de segurança
    if (setsockopt(this->server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)))
    {
        strerror(errno);
        close(this->server_fd);
        exit(EXIT_FAILURE);
    }

    if (bind(this->server_fd, (struct sockaddr *)&addr, addrlen) < 0)
    {
        strerror(errno);
        std::cout << "Error no bind" << std::endl;
        close(this->server_fd);
        exit(EXIT_FAILURE);
    }
}

void Server::startListenServer()
{

    if (listen(this->server_fd, 5) < 0)
    {
        strerror(errno);
        std::cout << "Error no listen" << std::endl;
        close(this->server_fd);
        exit(EXIT_FAILURE);
    }

    std::cout << "Servidor escutando na porta 4242..." << std::endl;
}

void Server::createEpoll()
{

    // preenche com dados o ev
    epoll_event ev;

    // cria o epoll,
    this->epoll_fd = epoll_create(1);
    // struct que o epoll preenche
    ev.events = EPOLLIN;
    ev.data.fd = this->server_fd;

    // adiciona o fd do server para ser monitorado pelo epoll
    epoll_ctl(this->epoll_fd, EPOLL_CTL_ADD, this->server_fd, &ev);
}

void Server::handleNewConnection()
{
    epoll_event ev;
    // aceita a nova conexão, pode passar uma struct sockaddr para o accept preencher com informções do cliente, como ip e porta que ele se conectou, e um socklen_t para saber também o tamanho da estrutura
    int client_fd = accept(this->server_fd, NULL, NULL);

    if (client_fd != -1)
    {

        // definir o fd como não bloqueante
        fcntl(client_fd, F_SETFL, O_NONBLOCK);
        // vamos reutilizar a estrutuara pra adicionar mais fds no epoll
        ev.events = EPOLLIN;
        ev.data.fd = client_fd;
        // adicionar o fd do cliente para o epoll monitorar se recebeu dados
        epoll_ctl(this->epoll_fd, EPOLL_CTL_ADD, client_fd, &ev);

        // Um unico cliente pode fazer 3 conexôes para obter mais dados de umavez
        std::cout << "Cliente conectado!" << std::endl;
    }
};

void Server::handleClientRequest(int client_fd)
{

    // cliente já conectado
    // não sei se o buffer sempre vai ter tamanho fixo
    char buffer[2048];
    // receber o que o cliente enviou, provavelmente está pedindo algo. Atribui a quantidade de bytes recebidos pelo cliente
    int bytes = recv(client_fd, buffer, sizeof(buffer), 0);

    if (bytes > 0)
    {
        // std::cout << "Recebido: \n" << std::string(buffer, bytes) << std::endl;

        HttpRequest request(buffer);

        // Pega o location referente ao que o cliente quer
        const Location &location = this->findLocation(request);

        // verificar se o metodo e permitido

        if (location.isMethodAllowed(request.getMethod()))
        {
            std::cout << "metodo permitido" << std::endl;
        }
        else
        {
            std::cout << "metodo não permitido" << std::endl;
        }

        // arquivo existe ?

        // Pegar o arquivo

        std::string bodyResponse = fileToString(location, request.getPath());

        // se o arquivo for script executa

        // se não existir coloca o not found

        // Construir o body

        // Construir todo response no HttpResponse
        // std::string body =
        // "<html>"
        //     "<head>"
        //         "<title>Teste Werbserv</title>"
        //         "<link rel='stylesheet' href='../src/style.css'>"
        //     "</head>"
        //     "<main>"
        //         "<h1>Hello Browser!</h1>"
        //     "</main>"
        // "</html>";

        std::string pthExtension = request.getPath();
        if (pthExtension == "/")
        {
            pthExtension = "index.html";
        }

        // Converter tamanho para string usando ostringstream
        std::ostringstream oss;
        oss << strlen(bodyResponse.c_str());

        // pega o tipo de arquivo, deve ser pegado antes para mudar a forma que enviar os dados para o cliente
        // esta colocando um arquivo para baixar
        std::string contentType = Utils::getContentType(pthExtension);

        std::string response = "HTTP/1.1 200 OK\r\n";
        response += ("Content-Type: " + contentType + "\r\n");
        response += "Content-Length: " + oss.str() + "\r\n";
        response += "\r\n"; // fim dos cabeçalhos
        response += bodyResponse;

        send(client_fd, response.c_str(), response.size(), 0);
        // manter keep-alive
        // response += "Connection: close\r\n";
    }
    else
    {

        // cliente se desconectou
        // fechar fd
        close(client_fd);

        // remover da lista de monitoramento do epoll
        epoll_ctl(this->epoll_fd, EPOLL_CTL_DEL, client_fd, NULL);
        std::cout << "Cliente desconectado." << std::endl;
    }
};

void Server::eventLoop()
{
    while (true)
    {
        // Fica esperando até ter algum evento em um dos fds monitorados, no inicio esta monitorando apenas o fd do server
        int n = epoll_wait(this->epoll_fd, this->events, 10, -1); // -1 serve para monitorar por tempo indeterminado

        // n retornar o numero de fds que teve eventos e preenche o array de events com os fds que recebeu os eventos

        // vamos rodar por todos os fds que o epoll disse que teve eventos
        for (int i = 0; i < n; i++)
        {

            int fd = this->events[i].data.fd;

            // se um dos fds que teve um evento foi o do server, então temos uma nova conexão
            if (fd == this->server_fd)
            {
                // aceitar nova conexão
                handleNewConnection();
            }
            else
            {
                handleClientRequest(fd);
            }
        }
    }
}

void Server::start()
{

    // inicia as configurações do socket
    initSocket();

    // permitir conecxões com o socket
    startListenServer();

    // cria o epoll, faz sentido ser a piscina de fds que irão se conectar, não sei por que o nome e poll
    createEpoll();

    // Lidar com os eventos de requisição
    eventLoop();
}
