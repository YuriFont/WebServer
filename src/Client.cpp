#include "../include/Client.hpp"
#include "../include/Utils.hpp"


Client::Client(const int& client_fd) {

    this->client_fd = client_fd;
    this->event.data.fd = client_fd;
    this->event.events = EPOLLIN;
};


epoll_event& Client::getDataEvent() {
    return this->event;
};

void Client::addBody(const std::string& request) {

    this->bodyRequest.append(request);
};