#ifndef CGIPROCESS_HPP
#define CGIPROCESS_HPP

#include <string>
#include <sys/types.h>

struct CgiProcess {
    int stdin_fd;
    int stdout_fd;
    pid_t pid;
    int client_fd;

    std::string input;
    std::string output;

    bool stdout_closed;
    bool stdin_closed;

    CgiProcess()
        : stdin_fd(-1),
          stdout_fd(-1),
          pid(-1),
          client_fd(-1),
          stdout_closed(false),
          stdin_closed(false) {}
};

#endif