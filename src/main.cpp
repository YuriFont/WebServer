/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yufonten <yufonten@student.42.rio>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/09 16:09:31 by yufonten          #+#    #+#             */
/*   Updated: 2025/09/10 10:44:01 by yufonten         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <iostream>
#include <cstring>
#include <cstdlib>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <cerrno>
#include <sstream> 

// Testando o funcionamento do socket
int main() {
    int server_fd, new_socket;
    struct sockaddr_in addr;
    int opt = 1;
    socklen_t addrlen = sizeof(addr);

    // Criando o socket (IPv4, TCP, protocolo padrão)
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1) {
        strerror(errno);
        exit(EXIT_FAILURE);
    }

    // Não entendi muito bem essa parte ainda kk
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
        strerror(errno);
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    // Configurando endereço (IPv4, qualquer IP local, porta 8080)
    std::memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(8080);

    if (bind(server_fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        strerror(errno);
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    // Escutar conexões (até 3 na fila)
    if (listen(server_fd, 3) < 0) {
        strerror(errno);
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    std::cout << "Servidor esperando conexões na porta 8080...\n";

    while (true) {
         new_socket = accept(server_fd, (struct sockaddr*)&addr, &addrlen);

        // Lê a requisição do browser
        char buffer[3000];
        int valread = recv(new_socket, buffer, sizeof(buffer)-1, 0);
        if (valread > 0) {
            buffer[valread] = '\0';
            std::cout << "Requisição recebida:\n" << buffer << "\n";
        }

        // Monta resposta HTTP
        const char* body = "<h1>Hello Browser!</h1>";
        
        // Converter tamanho para string usando ostringstream
        std::ostringstream oss;
        oss << strlen(body);
        
        std::string response = "HTTP/1.1 200 OK\r\n";
        response += "Content-Type: text/html\r\n";
        response += "Content-Length: " + oss.str() + "\r\n";
        response += "Connection: close\r\n";
        response += "\r\n"; // fim dos cabeçalhos
        response += body;

        // Envia resposta
        send(new_socket, response.c_str(), response.size(), 0);

        close(new_socket); // encerra conexão
    }

    return 0;
}