#ifndef CGIPROCESS_HPP
#define CGIPROCESS_HPP

#include <string>
#include <sys/types.h>

struct CgiProcess {

  enum CgiStatus {
        CGI_OK,             // 200 - Tudo certo
        CGI_NOT_FOUND,      // 404 - Arquivo não existe
        CGI_FORBIDDEN,      // 403 - Traversal path ou permissão
        CGI_INTERNAL_ERROR  // 500 - Falha no pipe/fork/exec
    };

    int stdin_fd;
    int stdout_fd;
    pid_t pid;
    int client_fd;
    size_t input_offset;

    std::string input;
    std::string output;

    bool stdout_closed;
    bool stdin_closed;

    CgiStatus   status;

    CgiProcess()
        : stdin_fd(-1),
          stdout_fd(-1),
          input_offset(0),
          pid(-1),
          client_fd(-1),
          stdout_closed(false),
          stdin_closed(false),
          status(CGI_OK) {}
};

#endif