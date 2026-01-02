import socket
import os
import mimetypes

HOST = "127.0.0.1"
PORT = 8080
PATH = "/upload"
BOUNDARY = "----WebKitFormBoundaryMultiChunk"


# Definir aquivos aqui
FILES = {
    "file1": "./tests/files/image.png",
    "file2": "./tests/files/test.json",
    "file2": "./tests/files/test.txt",
}

CHUNK_SIZE = 1024


def send_chunk(sock, data: bytes):
    sock.sendall(f"{len(data):X}\r\n".encode())
    sock.sendall(data)
    sock.sendall(b"\r\n")


def send_headers(sock):
    headers = (
        f"POST {PATH} HTTP/1.1\r\n"
        f"Host: {HOST}:{PORT}\r\n"
        f"Transfer-Encoding: chunked\r\n"
        f"Content-Type: multipart/form-data; boundary={BOUNDARY}\r\n"
        f"\r\n"
    )
    sock.sendall(headers.encode())


def send_file_part(sock, field_name, file_path):
    filename = os.path.basename(file_path)
    content_type = mimetypes.guess_type(file_path)[0] or "application/octet-stream"

    # boundary
    send_chunk(sock, f"--{BOUNDARY}\r\n".encode())

    # headers do arquivo
    send_chunk(
        sock,
        (
            f'Content-Disposition: form-data; name="{field_name}"; '
            f'filename="{filename}"\r\n'
            f"Content-Type: {content_type}\r\n\r\n"
        ).encode()
    )

    # conteúdo do arquivo (streaming)
    with open(file_path, "rb") as f:
        while True:
            data = f.read(CHUNK_SIZE)
            if not data:
                break
            send_chunk(sock, data)

    # fim da parte
    send_chunk(sock, b"\r\n")


def send_multipart(sock, files):
    for field_name, file_path in files.items():
        send_file_part(sock, field_name, file_path)

    # boundary final
    send_chunk(sock, f"--{BOUNDARY}--\r\n".encode())


def main():
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    sock.connect((HOST, PORT))

    send_headers(sock)
    send_multipart(sock, FILES)

    # fim do chunked
    sock.sendall(b"0\r\n\r\n")

    print(sock.recv(8192).decode(errors="ignore"))
    sock.close()


if __name__ == "__main__":
    main()