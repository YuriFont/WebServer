from utils.http_client import send_raw_http
from pathlib import Path

# Teste de envio post Multpart com diversos arquivos em chunked, na pasta /uploads deve possuir os seguintes arquivos
# test_chunked_multipart.txt
# image_chunked_multipart.png
# test_chunked_multipart.json
# webserv_chunked_multipart.pdf
# video_chunked_multipart.mp4
BASE_DIR = Path(__file__).resolve().parent.parent  # tests/
FILES_DIR = BASE_DIR / "files"
def test_post_multipart_multiple_real_files_chunked(server_addr):
    host, port = server_addr
    boundary = "BOUNDARY42"

    txt = (FILES_DIR / "test.txt").read_bytes()
    img = (FILES_DIR / "image.png").read_bytes()
    json_file = (FILES_DIR / "test.json").read_bytes()
    pdf = (FILES_DIR / "webserv-1.pdf").read_bytes()
    video = (FILES_DIR / "video.mp4")

    # Função auxiliar para criar um chunk
    def to_chunk(data: bytes) -> bytes:
        return hex(len(data))[2:].encode() + b"\r\n" + data + b"\r\n"

    chunks = []

    # file1 - txt
    chunks.append(to_chunk(
        f"--{boundary}\r\n"
        "Content-Disposition: form-data; name=\"file1\"; filename=\"test_chunked_multipart.txt\"\r\n"
        "Content-Type: text/plain\r\n\r\n".encode()
    ))
    chunks.append(to_chunk(txt))

    # file2 - image
    chunks.append(to_chunk(
        f"--{boundary}\r\n"
        "Content-Disposition: form-data; name=\"file2\"; filename=\"image_chunked_multipart.png\"\r\n"
        "Content-Type: image/png\r\n\r\n".encode()
    ))
    chunks.append(to_chunk(img))

    # file3 - json
    chunks.append(to_chunk(
        f"--{boundary}\r\n"
        "Content-Disposition: form-data; name=\"file3\"; filename=\"test_chunked_multipart.json\"\r\n"
        "Content-Type: application/json\r\n\r\n".encode()
    ))
    chunks.append(to_chunk(json_file))

    # file4 - pdf
    chunks.append(to_chunk(
        f"--{boundary}\r\n"
        "Content-Disposition: form-data; name=\"file4\"; filename=\"webserv_chunked_multipart.pdf\"\r\n"
        "Content-Type: application/pdf\r\n\r\n".encode()
    ))
    chunks.append(to_chunk(pdf))

    # file5 - video
    if (video.exists()):
        chunks.append(to_chunk(
            f"--{boundary}\r\n"
            "Content-Disposition: form-data; name=\"file5\"; filename=\"video_chunked_multipart.mp4\"\r\n"
            "Content-Type: video/mp4\r\n\r\n".encode()
        ))
        chunks.append(to_chunk(video.read_bytes()))

    # fechamento do multipart
    chunks.append(to_chunk(f"--{boundary}--\r\n".encode()))

    # chunk final 0
    chunks.append(b"0\r\n\r\n")

    # monta o corpo completo
    body = b"".join(chunks)

    # monta a requisição HTTP
    req = (
        b"POST /upload HTTP/1.1\r\n"
        b"Host: localhost\r\n"
        b"Connection: close\r\n"
        b"Transfer-Encoding: chunked\r\n"
        b"Content-Type: multipart/form-data; boundary=" + boundary.encode() + b"\r\n\r\n"
        + body
    )

    res = send_raw_http(req, host=host, port=port)

    assert b"200 OK" in res or b"201" in res