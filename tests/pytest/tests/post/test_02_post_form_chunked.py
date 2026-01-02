from utils.http_client import send_raw_http

#envio POST em chuncked, sem content length, na pasta uploads deve ter um .txt com "user=42&project=webserv&post=form"
def test_post_form_chunked(server_addr):
    host, port = server_addr
    data = b"user=42&project=webserv&post=form"

    chunk = hex(len(data))[2:].encode() + b"\r\n" + data + b"\r\n0\r\n\r\n"

    req = (
        b"POST /upload HTTP/1.1\r\n"
        b"Host: localhost\r\n"
        b"Connection: close\r\n"
        b"Transfer-Encoding: chunked\r\n"
        b"Content-Type: application/x-www-form-urlencoded\r\n\r\n"
        + chunk
    )

    res = send_raw_http(req, host=host, port=port)
    assert b"201" in res
