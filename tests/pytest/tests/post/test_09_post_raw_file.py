from utils.http_client import send_raw_http

# Um POST simple de arquivo binario, dentro da pasta /uploads deve ter um .bin com "HELLO BINARIO", se não suportado não cria nada, pois application/octet-stream não esta mapeado para salvar
def test_post_raw_file(server_addr):
    host, port = server_addr
    body = b"\x00\x01\x02\x03HELLO BINARIO"

    req = (
        b"POST /upload HTTP/1.1\r\n"
        b"Host: localhost\r\n"
        b"Connection: close\r\n"
        b"Content-Type: application/octet-stream\r\n"
        b"Content-Length: " + str(len(body)).encode() + b"\r\n\r\n"
        + body
    )

    res = send_raw_http(req, host=host, port=port)
    assert b"415" in res
