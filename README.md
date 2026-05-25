# WebServer

> A non-blocking, event-driven HTTP/1.1 server written from scratch in **C++98**, inspired by NGINX. Built as part of the **42 Network curriculum** (`webserv`).

<p align="center">
  <img alt="Language" src="https://img.shields.io/badge/C%2B%2B-98-00599C?style=for-the-badge&logo=cplusplus&logoColor=white">
  <img alt="Build" src="https://img.shields.io/badge/build-Makefile-064F8C?style=for-the-badge&logo=gnu&logoColor=white">
  <img alt="Platform" src="https://img.shields.io/badge/Linux-epoll-FCC624?style=for-the-badge&logo=linux&logoColor=black">
  <img alt="CI" src="https://img.shields.io/badge/CI-GitHub%20Actions-2088FF?style=for-the-badge&logo=githubactions&logoColor=white">
  <img alt="42" src="https://img.shields.io/badge/42-school-000?style=for-the-badge&logo=42&logoColor=white">
</p>

---

## TL;DR

A production-style HTTP/1.1 server that handles many simultaneous clients in a **single thread** using the Linux `epoll` API, parses an **NGINX-like configuration file**, executes **CGI scripts** (Python & PHP) over pipes with timeout protection, decodes **chunked transfer encoding** with a state machine, and streams **multipart/form-data uploads** through a clean **factory + strategy** pipeline — all within the strict subset of **C++98** (no `auto`, no `nullptr`, no STL post-98, no `select`/`poll` mixing).

---

## Why this project is interesting

This is not "a wrapper around `socket()`". It's a real server design exercise where every byte that crosses the network is parsed by code in this repo. Highlights worth a closer look:

- **Truly non-blocking I/O.** Every socket — listening, client, CGI pipe — is registered in a single `epoll` instance. There are no blocking reads or writes anywhere in the request path, and `read`/`write` return values are never used as a "did I block?" signal (per subject rules).
- **Hand-written HTTP/1.1 parser** with support for headers, `Content-Length`, `Transfer-Encoding: chunked`, query strings, malformed-request detection, and proper status-code mapping (400, 411, 413, 414, 501, 505, …).
- **Chunked decoder implemented as an explicit state machine** (`READ_SIZE → READ_DATA → DONE / ERROR`) so partial chunks split across multiple `read()` calls are handled correctly.
- **Body processing pipeline** built on a `IBodyProcessor` interface + `ABodyProcessor` abstract base, with concrete strategies for `multipart/form-data`, `application/x-www-form-urlencoded`, raw bodies, and a no-op processor — all instantiated by a `BodyProcessorFactory`.
- **Method handlers via polymorphism.** `GetHandler`, `PostHandler`, `DeleteHandler`, `CgiHandler`, `RedirectHandler`, `MethodNotAllowedHandler`, and `NotImplementedHandler` all implement a common `IMethodHandler` interface — the server doesn't know or care what's behind a route, it just calls `handleData()` and `getResponse()`.
- **CGI executed via `fork` + `pipe` + `execve`** with its stdin/stdout registered back into the same `epoll` loop, plus a 30-second watchdog that reaps runaway scripts (`while True: pass` ⇒ 504/500 instead of a wedged server).
- **Per-client idle timeout** (60 s by default) prevents slow-loris-style connection hoarding.
- **Graceful shutdown** on `SIGINT` — sockets are closed, clients are notified, no leaked file descriptors.
- **GitHub Actions CI** builds the server and runs the full test suite (GET / POST / DELETE / REDIRECT / CGI) on every push.

---

## Feature matrix

| Area | Supported |
|---|---|
| HTTP version | HTTP/1.1 |
| Methods | `GET`, `POST`, `DELETE` (plus `405`/`501` for the rest) |
| I/O model | Single-threaded, **`epoll`-based**, fully non-blocking |
| Config syntax | NGINX-style, multi-server, multi-port, multi-IP |
| Routing | Per-location `root`, `index`, allowed `methods`, `autoindex` |
| Redirects | Configurable per-location (`redirect <url>`) |
| Static files | With directory listing (`autoindex on`) |
| Uploads | `multipart/form-data` parsed and stored at `upload_store` |
| Request bodies | Raw, URL-encoded, multipart, **chunked**, ignored |
| Max body size | `client_max_body_size` enforced per server (→ `413`) |
| CGI | Python (`.py`) and PHP (`.php`) via `python3` / `php-cgi` |
| CGI safety | Per-process timeout, error reporting (404/403/500), zombie reaping |
| Error pages | Custom `error_page <code> <path>` per server |
| Connection mgmt | `Keep-Alive` & `Connection: close`, idle-client timeout |
| Signals | Graceful `SIGINT` shutdown |
| Security | Path traversal blocked, method-deny via `methods NONE` |

---

## Architecture at a glance

```
                       ┌────────────────────────────────┐
                       │           main.cpp             │
                       │  Config(file) → Server.start() │
                       └──────────────┬─────────────────┘
                                      │
                                      ▼
        ┌───────────────────────────────────────────────────────┐
        │                       Server                          │
        │   one epoll_fd, N listening sockets, M clients,       │
        │   K CGI pipes — all in the same event loop            │
        └───────────────────────────────────────────────────────┘
              │                  │                      │
              ▼                  ▼                      ▼
        ┌──────────┐      ┌────────────┐         ┌────────────┐
        │  Client  │ ───► │HttpRequest │ ──────► │  Router    │
        │  (fd,    │      │ (parser +  │         │ (Location  │
        │  buffer, │      │  chunked + │         │  lookup)   │
        │  state)  │      │  headers)  │         └─────┬──────┘
        └──────────┘      └────────────┘               │
                                                       ▼
                         ┌──────────────────────────────────────────┐
                         │       IMethodHandler  (Strategy)         │
                         │ ┌────────┐ ┌────────┐ ┌────────┐ ┌─────┐ │
                         │ │  Get   │ │  Post  │ │ Delete │ │ CGI │ │
                         │ └────────┘ └────────┘ └────────┘ └─────┘ │
                         │ ┌──────────┐ ┌───────────────┐ ┌───────┐ │
                         │ │ Redirect │ │ MethodNotAllow│ │ 501   │ │
                         │ └──────────┘ └───────────────┘ └───────┘ │
                         └──────────────────────────────────────────┘
                                                       │
                                                       ▼
                         ┌──────────────────────────────────────────┐
                         │     ABodyProcessor  (built by Factory)   │
                         │   Multipart │ UrlEncoded │ Raw │ Ignore  │
                         └──────────────────────────────────────────┘
```

Design patterns actually in use (not just buzzwords):

- **Strategy** — `IMethodHandler` lets the server treat every request type uniformly.
- **Factory** — `BodyProcessorFactory::createBodyProcessor(...)` picks the right body parser from headers + location config.
- **Template Method / ABC** — `ABodyProcessor` shares common boilerplate; concrete classes override only what they need.
- **State machine** — `ChunkedDecoder` for HTTP chunked bodies.
- **RAII** — every `fd`, every `CgiProcess`, every dynamically allocated handler has a clear owner and a clear destruction point.

---

## Build & run

### Requirements

- Linux (the server uses `epoll` and POSIX `fork`/`pipe`/`waitpid`)
- `g++` / `clang++` with C++98 support
- `make`
- *(Optional, for CGI)* `python3` at `/usr/bin/python3`, `php-cgi` at `/usr/bin/php-cgi`

### Compile

```bash
make            # builds bin/webserv with -Wall -Wextra -Werror -std=c++98
make re         # full rebuild
make clean      # remove object files
make fclean     # remove objects + binary
```

### Launch

```bash
./bin/webserv                       # uses config/default.conf
./bin/webserv config/av.conf        # custom config
```

Stop with `Ctrl+C` — the SIGINT handler closes all sockets cleanly.

### Try it

```bash
# Basic GET
curl -i http://localhost:8080/

# Upload via multipart/form-data
curl -i -F "file=@./somefile.txt" http://localhost:8080/upload

# Trigger CGI (Python)
curl -i http://localhost:8080/cgi-bin/hello.py

# Trigger a redirect
curl -i http://localhost:8080/oldpage

# Try to break it — header without ':' should return 400
printf "GET / HTTP/1.1\r\nHost: x\r\nBrokenHeader\r\n\r\n" | nc localhost 8080
```

---

## Configuration example

The parser accepts a familiar NGINX-style syntax — multiple `server` blocks, each with `location` blocks:

```nginx
server {
    listen localhost:8080;
    server_name my_server;

    error_page 404 errors/404.html;
    error_page 403 errors/403.html;

    client_max_body_size 105000000;

    location / {
        root www/html;
        index index.html index.htm;
        methods GET;
    }

    location /upload {
        root www/uploads;
        methods POST DELETE;
        upload_store www/uploads;
    }

    location /files {
        root www/files;
        autoindex on;
        methods GET;
    }

    location /oldpage {
        redirect https://google.com;
    }

    location /cgi-bin {
        root www/cgi-bin;
        methods GET POST DELETE;
        cgi .py  /usr/bin/python3;
        cgi .php /usr/bin/php-cgi;
    }

    location /secret {
        methods NONE;      # explicitly deny everything → 405
    }
}

server {
    listen 127.0.0.1:8082;
    server_name another_server;
    # ...another virtual host on a different port
}
```

---

## Project layout

```
WebServer/
├── src/
│   ├── main.cpp
│   ├── core/                 # Server / Client / ServerConfig / event loop
│   ├── config/               # NGINX-style config parser
│   ├── http/                 # HttpRequest / HttpResponse / HttpStatus
│   ├── handlers/             # GET / POST / DELETE / CGI / Redirect / 405 / 501
│   ├── bodyProcessor/        # Multipart / Raw / UrlEncoded / Chunked / Factory
│   ├── abstracts/            # ABodyProcessor (template method base)
│   └── utils/                # ErrorPage, helpers
├── include/                  # mirror of src/, public headers
├── config/                   # *.conf files (default + per-test)
├── errors/                   # custom 403 / 404 pages
├── tests/                    # bash + pytest test suites
├── www/                      # site root used by default.conf
├── .github/workflows/        # CI pipeline
└── Makefile
```

---

## Testing

Two complementary test suites live under `tests/`:

**1. Bash integration tests** — fast smoke tests per HTTP method, runnable individually or via `make test`:

```bash
make test
# or, individually:
cd tests
bash get_test.sh
bash post_test.sh
bash delete_test.sh
bash redirect_test.sh
bash cgi_test.sh
bash permission_cgi_test.sh
```

**2. Pytest suite** (`tests/pytest/`) — heavier scenarios: chunked uploads up to **50–100 MB**, fragmented chunked POSTs, multipart with multiple files, status-code edge cases, idle-client behavior, CGI over chunked.

Both suites are wired into **GitHub Actions** (`.github/workflows/webserv-tests.yml`) and run on every push to `main`.

---

## Notable engineering details

- **One `epoll_event` loop, three kinds of fds.** Listening sockets, client sockets, and CGI pipes all flow through the same `epoll_wait` — there is no second event loop, no thread pool, no `select` fallback.
- **CGI is fully async.** The server `fork`s, `execve`s the interpreter, sets the pipe ends non-blocking, registers them in `epoll`, and **does not block** waiting for the script to finish. A `last_activity` timestamp + `CGI_TIMEOUT` guard catches infinite loops without polling.
- **No memory leaks tolerated.** Every `new` has a corresponding `delete`; handlers are owned by `Client`, body processors by handlers, CGI processes by the server map. `make` builds with `-Werror`.
- **C++98 strictness.** No `auto`, no range-`for`, no `<unordered_map>`, no smart pointers, no `std::thread`. The codebase reflects what disciplined pre-C++11 server code looks like.
- **Cross-method consistency.** Methods that aren't enabled for a location go through `MethodNotAllowedHandler` instead of an `if` chain — adding a new HTTP verb later is a matter of writing one class.

---

## Authors

This is a **42 group project** delivered as part of the *webserv* assignment. Code in this repository was written and is maintained by:

- **Yuri Fontenele** — [GitHub](https://github.com/YuriFont) · yufonten@student.42.rio

Built at **42 Rio**.

---

<details>
<summary><strong>Versão em Português 🇧🇷</strong> (clique para expandir)</summary>

## WebServer — visão geral

Servidor HTTP/1.1 **não-bloqueante** e **orientado a eventos** escrito do zero em **C++98**, inspirado no NGINX. Projeto do currículo da **42** (`webserv`).

### Por que vale dar uma olhada

- **I/O 100% não-bloqueante** com `epoll`: um único loop cuida de sockets de escuta, conexões de clientes e pipes de CGI ao mesmo tempo.
- **Parser HTTP/1.1 feito à mão**, com suporte a headers, `Content-Length`, `Transfer-Encoding: chunked`, query string e detecção de requisições malformadas.
- **Decodificador `chunked`** implementado como **máquina de estados explícita**, lidando corretamente com chunks fragmentados em vários `read()`.
- **Pipeline de body** com `IBodyProcessor` + `ABodyProcessor` e estratégias concretas para `multipart/form-data`, `x-www-form-urlencoded`, body raw e "ignorar", todas montadas por uma **Factory**.
- **Handlers por método** (`GET`, `POST`, `DELETE`, `CGI`, `Redirect`, `405`, `501`) implementam a mesma interface `IMethodHandler` — o servidor não precisa saber o que cada rota faz por baixo.
- **CGI** (Python e PHP) executado via `fork` + `pipe` + `execve`, registrado no mesmo `epoll`, com **timeout de 30 s** para matar scripts em loop infinito.
- **Timeout por cliente** (60 s) evita conexões zumbis.
- **Shutdown gracioso** via `SIGINT` — nada de fd vazado.
- **CI no GitHub Actions** roda toda a suíte (bash + pytest) a cada push.

### Configuração estilo NGINX

Múltiplos blocos `server`, múltiplas portas, blocos `location` com `root`, `index`, `methods`, `autoindex`, `redirect`, `upload_store`, `cgi`, e `client_max_body_size`. Veja `config/default.conf`.

### Como rodar

```bash
make
./bin/webserv config/default.conf
# em outro terminal:
curl -i http://localhost:8080/
```

### Testes

```bash
make test                  # roda toda a suíte bash
cd tests/pytest && pytest  # cenários pesados (chunked 50/100 MB, multipart, CGI)
```

### Autores

Projeto em equipe da **42** — código mantido por **Yuri Fontenele** ([GitHub](https://github.com/YuriFont)).

</details>
