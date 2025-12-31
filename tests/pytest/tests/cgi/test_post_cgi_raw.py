from utils.http_client import send_raw_http

#Test POST para o CGI, o servidor deve processar o CGI e devolver o corpo mesmo que foi enviado
def test_post_cgi_raw(server_addr):
    host, port = server_addr
    body = b"hello=42"

    req = (
        b"POST /cgi-bin/test.py HTTP/1.1\r\n"
        b"Host: localhost\r\n"
        b"Connection: close\r\n"
        b"Content-Length: " + str(len(body)).encode() + b"\r\n\r\n"
        + body
    )

    res = send_raw_http(req, host=host, port=port)
    assert b"200" in res
    assert b"hello=42" in res
