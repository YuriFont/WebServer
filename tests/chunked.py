import socket

HOST = "127.0.0.1"
PORT = 8080
PATH = "/cgi-bin/cgi-chunked.py"

def send_chunk(sock, data: bytes):
    sock.sendall(f"{len(data):X}\r\n".encode())
    sock.sendall(data)
    sock.sendall(b"\r\n")

sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
sock.connect((HOST, PORT))

headers = (
    f"POST {PATH} HTTP/1.1\r\n"
    f"Host: {HOST}:{PORT}\r\n"
    f"Transfer-Encoding: chunked\r\n"
    f"Content-Type: text/plain\r\n"
    f"\r\n"
)
sock.sendall(headers.encode())

# chunks
send_chunk(sock, b"hello ")
send_chunk(sock, b"from ")
send_chunk(sock, b"chunked ")
send_chunk(sock, b"cgi\n")

# fim
sock.sendall(b"0\r\n\r\n")

print(sock.recv(8192).decode(errors="ignore"))
sock.close()