from utils.http_client import send_raw_http

#Teste GET em CGI, o servidor deve devolver 200 OK
def test_get_cgi(server_addr):
    host, port = server_addr
    req = (
        b"GET /cgi-bin/test_cgi.py?name=42 HTTP/1.1\r\n"
        b"Connection: close\r\n"
        b"Host: localhost\r\n"
        b"\r\n"
    )

    res = send_raw_http(req, host=host, port=port)

    assert b"200 OK" in res
    assert b"Content-Type" in res
