from utils.http_client import send_raw_http

# teste POST puro, com o servidor salvando o arquivo com base no content-type, dentro do arquivo deve ter "HELLO"
def test_post_raw_chunked(server_addr):
    host, port = server_addr
    data = b"\x00\xff\x01\xfeHELLO"

    chunk = hex(len(data))[2:].encode() + b"\r\n" + data + b"\r\n0\r\n\r\n"

    req = (
        b"POST /upload HTTP/1.1\r\n"
        b"Host: localhost\r\n"
        b"Connection: close\r\n"
        b"Transfer-Encoding: chunked\r\n"
        b"\r\n" + chunk
    )

    res = send_raw_http(req, host=host, port=port)
    assert b"201" in res
