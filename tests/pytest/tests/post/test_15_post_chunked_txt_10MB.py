from pathlib import Path
from utils.http_client import send_raw_http
from utils.ensure_file_exist import ensure_test_file

# arquivo de 10MB enviado para o servidor em chunked(pedaços, não sabemos o content-length), um arquivo .txt de 10MB será criado na pasta /uploads
BASE_DIR = Path(__file__).resolve().parent.parent  # tests/
FILES_DIR = BASE_DIR / "files"
def test_post_raw_existing_txt_10mb_chunked(server_addr):
    host, port = server_addr
    # Criar arquivo
    # dd if=/dev/zero of=text_10mb.txt bs=1024 count=10240
    file_path = (FILES_DIR / "text_10mb.txt")
    ensure_test_file(file_path, size_mb=10, pattern=b"X")

    chunk_size = 8192  # 8KB por chunk (bom equilíbrio)
    nb = 0
    chunks = []

    with file_path.open("rb") as f:
        while True:
            part = f.read(chunk_size)
            if not part:
                break

            nb += 1
            chunks.append(
                hex(len(part))[2:].encode() + b"\r\n" +
                part + b"\r\n"
            )

    print(f"Total de chunks enviados: {nb}")

    # chunk final
    chunks.append(b"0\r\n\r\n")

    body = b"".join(chunks)

    req = (
        b"POST /upload/ HTTP/1.1\r\n"
        b"Host: localhost\r\n"
        b"Connection: close\r\n"
        b"Transfer-Encoding: chunked\r\n"
        b"Content-Type: text/plain\r\n\r\n"
        + body
    )

    res = send_raw_http(req, host=host, port=port)

    assert b"200 OK" in res or b"201" in res