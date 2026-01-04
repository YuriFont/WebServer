# tests./test_status_codes.py
from utils.http import get, post
from utils.http_client import send_raw_http

import socket
import time

# executar um teste em especifico
# pytest tests/codes/test_status_codes.py::<nome da função>
# -vv -> bem detalhado
# -s -> permite printar no terminal
def test_200_ok(server_addr):
    host, port = server_addr
    r = get("/", host=host,port=port)
    assert r.status_code == 200

def test_404_not_found(server_addr):
    host, port = server_addr
    r = get("/arquivo_que_nao_existe.html", host=host,port=port)
    assert r.status_code == 404

def test_405_method_not_allowed(server_addr):
    host, port = server_addr
    r = post("/",host=host, port=port)
    assert r.status_code == 405
    
    
# RFC-compliant servers PODEM e DEVEM:
# - Ler só os headers
# - Decidir 413
# - Responder
# - Encerrar a conexão imediatamente
# pytest tests/codes/test_status_codes.py::test_413_payload_too_large
def test_413_payload_too_large(server_addr):
    host, port = server_addr
    MB = 1024 * 1024 # 1MB
    max_body_mb = 51 # Este valor vai ser multiplicado por 1mb; DEVE bater com o .conf
    big_body = b"a" * ((max_body_mb + 1) * MB)  # maior que max_body_size
    
    request = (
        b"POST /upload HTTP/1.1\r\n"
        b"Host: localhost\r\n"
        b"Content-Length: " + str(len(big_body)).encode() + b"\r\n"
        b"\r\n"
    )
    r = send_raw_http(request, host=host, port=port)
    
    assert b"413" in r

## Testa o 413 em um chunked(quando não se tem content length)
def test_413_payload_too_large_chunked(server_addr):
    host, port = server_addr
    MB = 1024 * 1024
    max_body_mb = 51  # DEVE bater com o .conf
    chunk_size = 8192

    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.settimeout(0.01)
    s.connect((host, port))

    header = (
        b"POST /upload HTTP/1.1\r\n"
        b"Host: localhost\r\n"
        b"Transfer-Encoding: chunked\r\n"
        b"Connection: close\r\n\r\n"
    )
    s.sendall(header)

    sent_bytes = 0
    response = b""

    try:
        while sent_bytes <= (max_body_mb + 1) * MB:
            data = b"a" * chunk_size
            size_hex = hex(len(data))[2:].encode()

            # size
            s.sendall(size_hex + b"\r\n")
            # data
            s.sendall(data + b"\r\n")

            sent_bytes += len(data)

            # tenta capturar resposta antecipada
            try:
                part = s.recv(8192)
                if part:
                    response += part
                    break
            except socket.timeout:
                pass

        # chunk final (APENAS UMA VEZ)
        s.sendall(b"0\r\n\r\n")
        
        ## esperar resposta do servidor
        for _ in range(500):
            try:
                part = s.recv(8192)
                if part:
                    response += part
                    break
            except socket.timeout:
                pass

    except (BrokenPipeError, ConnectionResetError):
        # servidor abortou cedo → esperado
        pass
    finally:
        s.close()

    print(response.decode(errors="ignore"))
    print(response)

    assert b"413" in response

def test_400_bad_request(server_addr):
    host, port = server_addr
    request = (
        b"POST / HTTP/1.1\r\n"
        b"Host:\r\n"
        b"\r\n"
    )

    response = send_raw_http(request, host=host, port=port)
    assert b"400" in response
    
def test_400_bad_request_method(server_addr):
    host, port = server_addr
    request = (
        b"PO ST / HTTP/1.1\r\n"
        b"Host: localhost\r\n"
        b"\r\n"
    )

    response = send_raw_http(request, host=host, port=port)
    assert b"400" in response
    

# 403 necessário modificar o Path para poder criar o arquivo secret.html dentro de html/
# def test_403_forbidden(tmp_path):
#     secret = Path("tests./conf/www/secret.html")
#     secret.write_text("secret")
#     secret.chmod(0o000)

#     r = get("/secret.html")
#     assert r.status_code == 403

#     secret.chmod(0o644)