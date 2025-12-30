#ifndef WEBSERVER_HPP
#define WEBSERVER_HPP

#include <iostream>
#include <fstream>
#include <string>
#include <cstring>
#include <cstdlib>
#include <fcntl.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <cerrno>
#include <sstream>
#include <map>
#include <vector>
#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>
#include <arpa/inet.h>
#include <ctime>
#include <dirent.h>
#include <sys/wait.h>
#include <csignal>

#define RESET   "\033[0m"
#define RED     "\033[31m"
#define GREEN   "\033[32m"
#define YELLOW  "\033[33m"
#define MAGENTA "\033[35m"
#define CYAN    "\033[36m"

#define CLIENT_TIMEOUT 60
#define CGI_TIMEOUT 30

#endif
