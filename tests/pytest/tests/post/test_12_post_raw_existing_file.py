from utils.http_client import send_raw_http
from pathlib import Path

# Teste de envio simples de uma imagem .png deve estar dentro da pasta /uploads com prefixo upload_ + numero aleatorio
BASE_DIR = Path(__file__).resolve().parent.parent  # tests/
FILES_DIR = BASE_DIR / "files"
def test_post_raw_existing_file(server_addr):
    host, port = server_addr
    data = (FILES_DIR / "image.png").read_bytes()

    req = (
        b"POST /upload/image.png HTTP/1.1\r\n"
        b"Host: localhost\r\n"
        b"Connection: close\r\n"
        b"Content-Type: image/png\r\n"
        b"Content-Length: " + str(len(data)).encode() + b"\r\n\r\n"
        + data
    )

    res = send_raw_http(req, host=host, port=port)

    assert b"200 OK" in res or b"201" in res