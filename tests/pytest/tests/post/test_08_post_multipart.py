from utils.http_client import send_raw_http

# teste POST multipart o servidor deve criar um arquivo hello42.txt com o seguinte texto dentro "hello 42"
def test_post_multipart(server_addr):
    host, port = server_addr
    boundary = b"----WebKitFormBoundary42"

    body = (
        b"--" + boundary + b"\r\n"
        b"Content-Disposition: form-data; name=\"file\"; filename=\"hello42.txt\"\r\n"
        b"Content-Type: text/plain\r\n\r\n"
        b"hello 42\r\n"
        b"--" + boundary + b"--\r\n"
    )

    req = (
        b"POST /upload HTTP/1.1\r\n"
        b"Host: localhost\r\n"
        b"Connection: close\r\n"
        b"Content-Type: multipart/form-data; boundary=" + boundary + b"\r\n"
        b"Content-Length: " + str(len(body)).encode() + b"\r\n\r\n"
        + body
    )

    res = send_raw_http(req, host=host, port=port)
    assert b"201" in res or b"200 OK" in res
