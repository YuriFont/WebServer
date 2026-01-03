from utils.http_client import send_raw_http
from utils.builder import build_multipart
from pathlib import Path

# Teste de envio post Multpart com diversos arquivos, na pasta /uploads deve possuir os seguintes arquivos
# test_raw_multipart.txt
# image_raw_multipart.png
# test_raw_multipart.json
# webserv_raw_multipart.pdf
# video_raw_multipart.mp4
BASE_DIR = Path(__file__).resolve().parent.parent  # tests/
FILES_DIR = BASE_DIR / "files"
def test_post_multipart_multiple_real_files(server_addr):
    host, port = server_addr
    boundary = b"BOUNDARY42"

    txt = (FILES_DIR / "test.txt").read_bytes()
    img = (FILES_DIR / "image.png").read_bytes()
    json = (FILES_DIR / "test.json").read_bytes()
    pdf = (FILES_DIR / "webserv-1.pdf").read_bytes()
    video = (FILES_DIR / "video.mp4")
    
    parts = [
        {
            "name": b"file1",
            "filename": b"test_raw_multipart.txt",
            "content": txt,
            "type": b"text/plain",
        },
        {
            "name": b"file2",
            "filename": b"image_raw_multipart.png",
            "content": img,
            "type": b"image/png",
        },
        {
            "name": b"file3",
            "filename": b"test_raw_multipart.json",
            "content": json,
            "type": b"application/json",
        },
        {
            "name": b"file4",
            "filename": b"webserv_raw_multipart.pdf",
            "content": pdf,
            "type": b"application/pdf",
        }
    ]
    
    if video.exists():
        parts.append({
            "name": b"file",
            "filename": b"video_raw_multipart.mp4",
            "content": video.read_bytes(),
            "type": b"video/mp4",
    })
        
    body = build_multipart(boundary=boundary, parts=parts)

    req = (
        b"POST /upload HTTP/1.1\r\n"
        b"Host: localhost\r\n"
        b"Connection: close\r\n"
        b"Content-Type: multipart/form-data; boundary=" + boundary + b"\r\n"
        b"Content-Length: " + str(len(body)).encode() + b"\r\n\r\n"
        + body
    )

    res = send_raw_http(req, host=host, port=port)

    assert b"200 OK" in res or b"201" in res