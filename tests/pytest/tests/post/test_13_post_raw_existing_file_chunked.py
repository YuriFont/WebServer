from pathlib import Path
from utils.http_client import send_raw_http

# Teste de envio simples de uma imagem .png porem em chuncked(pedaços sem content length), deve estar dentro da pasta /uploads com prefixo upload_ + numero aleatorio

BASE_DIR = Path(__file__).resolve().parent.parent  # tests/
FILES_DIR = BASE_DIR / "files"
def test_post_raw_existing_file_chunked_split(server_addr):
    host, port = server_addr
    data = (FILES_DIR / "image.png").read_bytes()
    chunk_size = 2048  # 1KB por chunk
    nb = 0

    chunks = []
    for i in range(0, len(data), chunk_size):
        nb = nb + 1
        part = data[i:i+chunk_size]
        chunks.append(hex(len(part))[2:].encode() + b"\r\n" + part + b"\r\n")

    # chunk final
    print(f"Total de chuncks {nb}")
    chunks.append(b"0\r\n\r\n")

    body = b"".join(chunks)

    req = (
        b"POST /upload/ HTTP/1.1\r\n"
        b"Host: localhost\r\n"
        b"Connection: close\r\n"
        b"Transfer-Encoding: chunked\r\n"
        b"Content-Type: image/png\r\n\r\n"
        + body
    )

    res = send_raw_http(req, host=host, port=port)

    assert b"200 OK" in res or b"201" in res
