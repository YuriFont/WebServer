from utils.http_client import send_raw_http

# Faz uma requisição para o CGI, um test.py que mostra o metodo feito e a query string se tiver
def test_post_cgi_chunked(server_addr):
    host, port = server_addr
    data = b"chunked=cgi"

    chunk = hex(len(data))[2:].encode() + b"\r\n" + data + b"\r\n0\r\n\r\n"

    req = (
        b"POST /cgi-bin/test.py HTTP/1.1\r\n"
        b"Host: localhost\r\n"
        b"Connection: close\r\n"
        b"Transfer-Encoding: chunked\r\n\r\n"
        + chunk
    )

    res = send_raw_http(req, host=host, port=port)
    assert b"200" in res
    assert b"chunked=cgi" in res
