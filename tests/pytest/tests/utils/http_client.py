import socket

def send_raw_http(request: bytes, host="127.0.0.1", port=8080) -> bytes:
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.connect((host, port))

    response = b""
    try:
        s.sendall(request)
        while True:
            chunk = s.recv(4096)
            if not chunk:
                break
            response += chunk
    except ConnectionResetError:
        # servidor fechou cedo (esperado em 413)
        pass
    finally:
        s.close()

    return response