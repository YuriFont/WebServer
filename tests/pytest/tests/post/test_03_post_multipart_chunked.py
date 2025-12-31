from utils.http_client import send_raw_http

#teste de POST em chuncked, na pasta uploads deve ter uma arquivo chamado helloChunked.txt com "hello chunked" dentro
def test_post_multipart_chunked(server_addr):
    host, port = server_addr
    boundary = "BOUND42"

    data = (
        f"--{boundary}\r\n"
        "Content-Disposition: form-data; name=\"file\"; filename=\"helloChunked.txt\"\r\n"
        "Content-Type: text/plain\r\n\r\n"
        "hello chunked\r\n"
        f"--{boundary}--\r\n"
    ).encode()

    chunks = [
        hex(len(data))[2:].encode() + b"\r\n" + data + b"\r\n",
        b"0\r\n\r\n"
    ]

    req = (
        b"POST /upload HTTP/1.1\r\n"
        b"Host: localhost\r\n"
        b"Connection: close\r\n"
        b"Transfer-Encoding: chunked\r\n"
        b"Content-Type: multipart/form-data; boundary=" + boundary.encode() + b"\r\n\r\n"
        + b"".join(chunks)
    )

    res = send_raw_http(req, host=host, port=port)
    assert b"201" in res
