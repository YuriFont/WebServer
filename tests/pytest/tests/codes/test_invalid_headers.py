from utils.http_client import send_raw_http

def test_header_without_colon(server_addr):
    host, port = server_addr
    req = (
        b"GET / HTTP/1.1\r\n"
        b"Host localhost\r\n"
        b"\r\n"
    )
    res = send_raw_http(req, host=host, port=port)
    assert b"400" in res