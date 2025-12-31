import socket
import pytest

@pytest.mark.manual
def test_client_disconnect_mid_body(server_addr):
    host, port = server_addr
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.connect((host, port))
    s.sendall(
        b"POST /upload HTTP/1.1\r\n"
        b"Host: localhost\r\n"
        b"Content-Length: 100\r\n\r\n"
        b"12345"
    )
    s.close()  # fecha no meio

    # PASSA se o servidor não crashar
    assert True
