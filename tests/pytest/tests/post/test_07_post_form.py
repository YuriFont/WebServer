from utils.http_client import send_raw_http

#Teste post de fomulario, no servidor na pasta /uploads deve ter um arquivo .txt com "user=erick&lang=cpp" dentro
def test_post_form(server_addr):
    host, port = server_addr
    body = b"user=erick&lang=cpp"

    req = (
        b"POST /upload HTTP/1.1\r\n"
        b"Host: localhost\r\n"
        b"Connection: close\r\n"
        b"Content-Type: application/x-www-form-urlencoded\r\n"
        b"Content-Length: " + str(len(body)).encode() + b"\r\n\r\n"
        + body
    )

    res = send_raw_http(req, host=host, port=port)
    assert b"201" in res
