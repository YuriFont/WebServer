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

Server::Server(const Config &config) : _config(config) {}

Server::~Server() {}

void Server::start() {
    
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1) {
        strerror(errno);
        exit(EXIT_FAILURE);
    }

    // não bloqueante, serve para não ficar esperando quando tiver em uma conexão, F_SETFL serve para modificar o descritor de arquivos
    fcntl(server_fd, F_SETFL, O_NONBLOCK);

    struct sockaddr_in addr;
    int opt = 1;
    socklen_t addrlen = sizeof(addr);

    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(4242);

    // permitir reuso da porta sem o timeout de segurança
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
        strerror(errno);
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    if (bind(server_fd, (struct sockaddr*)&addr, addrlen) < 0) {
        strerror(errno);
        std::cout << "Error no bind" << std::endl;
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 5) < 0) {
        strerror(errno);
        std::cout << "Error no listen" << std::endl;
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    std::cout << "Servidor escutando na porta 4242..." << std::endl;

    // cria o epoll, 
    int epfd = epoll_create(1);

    // struct que o epoll preenche
    epoll_event ev, events[10];
    ev.events = EPOLLIN;
    ev.data.fd = server_fd;

    // adiciona o fd do server para ser monitorado pelo epoll
    epoll_ctl(epfd, EPOLL_CTL_ADD, server_fd, &ev);

    while (true) {
        // Fica esperando até ter algum evento em um dos fds monitorados, no inicio esta monitorando apenas o fd do server
        int n = epoll_wait(epfd, events, 10, -1); // -1 serve para monitorar por tempo indeterminado

        // n retornar o numero de fds que teve eventos e preenche o array de events com os fds que recebeu os eventos

        // vamos rodar por todos os fds que o epoll disse que teve eventos
        for (int i = 0; i < n; i++) {

            int fd = events[i].data.fd;

            // se um dos fds que teve um evento foi o do server, então temos uma nova conexão
            if (fd == server_fd) {

                // aceita a nova conexão, pode passar uma struct sockaddr para o accept preencher com informções do cliente, como ip e porta que ele se conectou, e um socklen_t para saber também o tamanho da estrutura
                int client_fd = accept(server_fd, NULL, NULL);

                if (client_fd != -1) {

                    // definir o fd como não bloqueante
                    fcntl(client_fd, F_SETFL, O_NONBLOCK);
                    // vamos reutilizar a estrutuara pra adicionar mais fds no epoll
                    ev.events = EPOLLIN;
                    ev.data.fd = client_fd;
                    // adicionar o fd do cliente para o epoll monitorar se recebeu dados
                    epoll_ctl(epfd, EPOLL_CTL_ADD, client_fd, &ev);

                    // Um unico cliente pode fazer 3 conexôes para obter mais dados de umavez
                    std::cout << "Cliente conectado!" << std::endl;
                }
            }
            else {
                // cliente já conectado
                // não sei se o buffer sempre vai ter tamanho fixo
                char buffer[2048];
                // receber o que o cliente enviou, provavelmente está pedindo algo. Atribui a quantidade de bytes recebidos pelo cliente
                int bytes = recv(fd, buffer, sizeof(buffer), 0);


                if (bytes > 0) {
                    // std::cout << "Recebido: \n" << std::string(buffer, bytes) << std::endl;

                    HttpRequest request(buffer);

                    // Construir todo response no HttpResponse
                    std::string body = 
                    "<html>"
                        "<head>"
                            "<title>Teste Werbserv</title>"
                            "<link rel='stylesheet' href='../src/style.css'>"
                        "</head>"
                        "<main>"
                            "<h1>Hello Browser!</h1>"
                        "</main>"
                    "</html>";

        
                    // Converter tamanho para string usando ostringstream
                    std::ostringstream oss;
                    oss << strlen(body.c_str());

                    std::string response = "HTTP/1.1 200 OK\r\n";
                    response += "Content-Type: text/html\r\n";
                    response += "Content-Length: " + oss.str() + "\r\n";
                    response += "\r\n"; // fim dos cabeçalhos
                    response += body;

                    send(fd, response.c_str(), response.size(), 0);
                    // manter keep-alive
                    // response += "Connection: close\r\n";
                } else {

                    // cliente se desconectou
                    // fechar fd
                    close(fd);

                    // remover da lista de monitoramento do epoll
                    epoll_ctl(epfd, EPOLL_CTL_DEL, fd, NULL);
                    std::cout << "Cliente desconectado." << std::endl;
                }
            } 
        }
    }
}
