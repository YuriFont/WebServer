import socket
import time
from pathlib import Path

def test_post_chunked_split_crlf():
    file_path = Path("www/files/text_10mb.txt")
    upload_path = Path("www/uploads")

    if not file_path.exists():
        file_path.write_bytes(b"A" * 10 * 1024 * 1024)

    chunk_size = 8192

    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.connect(("localhost", 8080))

    header = (
        b"POST /upload/ HTTP/1.1\r\n"
        b"Host: localhost\r\n"
        b"Connection: close\r\n"
        b"Transfer-Encoding: chunked\r\n"
        b"Content-Type: text/plain\r\n\r\n"
    )
    s.sendall(header)

    with file_path.open("rb") as f:
        while True:
            part = f.read(chunk_size)
            if not part:
                break

            size_hex = hex(len(part))[2:].encode()

            s.send(size_hex + b"\r")
            time.sleep(0.001)
            s.send(b"\n")
            s.send(part)
            s.send(b"\r")
            s.send(b"\n")

    s.send(b"0\r\n\r\n")

    response = b""
    while True:
        data = s.recv(4096)
        if not data:
            break
        response += data

    s.close()

    print(response.decode(errors="ignore"))
    assert b"200" in response or b"201" in response

    # valida conteúdo
    uploaded_files = sorted(upload_path.glob("upload_*"))
    assert uploaded_files, "Nenhum arquivo foi criado"

    uploaded = uploaded_files[-1].read_bytes()
    original = file_path.read_bytes()

    assert uploaded == original, "Conteúdo do arquivo não bate"

if __name__ == "__main__":
    test_post_chunked_split_crlf()
