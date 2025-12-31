from utils.http_client import send_raw_http

#Teste GET simples na pagina inicial
def test_get_static_file(server_addr):
    host, port = server_addr
    req = (
        b"GET /index.html HTTP/1.1\r\n"
        b"Connection: close\r\n"
        b"Host: localhost\r\n"
        b"\r\n"
    )

    res = send_raw_http(req, host=host, port=port)

    assert b"200 OK" in res
    assert b"<html" in res
