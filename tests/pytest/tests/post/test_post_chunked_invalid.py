from utils.http_client import send_raw_http

def test_chunk_size_not_hex(server_addr):
    host, port = server_addr
    req = (
        b"POST /upload HTTP/1.1\r\n"
        b"Host: localhost\r\n"
        b"Transfer-Encoding: chunked\r\n\r\n"
        b"Z\r\n"
        b"abc\r\n"
        b"0\r\n\r\n"
    )
    res = send_raw_http(req, host=host, port=port)
    assert b"400" in res
