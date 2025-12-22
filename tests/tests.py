import requests
from requests_toolbelt import MultipartEncoder
from requests_toolbelt.streaming_iterator import StreamingIterator
import os
import time
import json

BASE_URL = "http://localhost:8080"
UPLOAD_PATH = "/upload"
DELETE_PATH = "/upload/delete_me.txt"
CGI_URL = "http://localhost:8080/cgi-bin/test_cgi.py"

FILES_DIR = "files"


# ---------- Utils ----------

def print_result(name, r):
    print(f"\n=== {name} ===")
    print("Status:", r.status_code)
    print("Headers:", dict(r.headers))
    print("Body:", r.text[:400])


def assert_ok(r):
    if r.status_code >= 400:
        print("❌ Test failed")
    else:
        print("✅ OK")


# ---------- GET ----------

def test_get():
    r = requests.get(f"{BASE_URL}/")
    print_result("GET /", r)
    assert_ok(r)


# ---------- RAW (vários tipos) ----------

def test_post_raw_text():
    data = b"plain text body\nsecond line\n"
    r = requests.post(
        f"{BASE_URL}/{UPLOAD_PATH}",
        data=data,
        headers={
            "Content-Type": "text/plain",
            "Content-Length": str(len(data))
        }
    )
    print_result("POST raw text", r)
    assert_ok(r)


def test_post_raw_json():
    payload = {"name": "erick", "project": "webserv"}
    data = json.dumps(payload).encode()

    r = requests.post(
        f"{BASE_URL}/{UPLOAD_PATH}",
        data=data,
        headers={
            "Content-Type": "application/json",
            "Content-Length": str(len(data))
        }
    )
    print_result("POST raw json", r)
    assert_ok(r)


def test_post_raw_binary():
    path = os.path.join(FILES_DIR, "test.bin")
    data = open(path, "rb").read()

    r = requests.post(
        f"{BASE_URL}/{UPLOAD_PATH}",
        data=data,
        headers={
            "Content-Type": "application/octet-stream",
            "Content-Length": str(len(data))
        }
    )
    print_result("POST raw binary", r)
    assert_ok(r)


# ---------- RAW CHUNKED ----------

def test_post_raw_chunked():
    def gen():
        yield b"chunk-1\n"
        time.sleep(0.1)
        yield b"chunk-2\n"
        time.sleep(0.1)
        yield b"chunk-3\n"

    r = requests.post(
        f"{BASE_URL}/{UPLOAD_PATH}",
        data=gen(),
        headers={
            "Content-Type": "text/plain",
            "Transfer-Encoding": "chunked"
        }
    )

    print_result("POST raw chunked", r)
    assert_ok(r)


# ---------- MULTIPART (1 arquivo) ----------

def test_multipart_single_file():
    path = os.path.join(FILES_DIR, "test.txt")

    m = MultipartEncoder(
        fields={
            "description": "single file",
            "file": ("test.txt", open(path, "rb"), "text/plain")
        }
    )

    r = requests.post(
        f"{BASE_URL}{UPLOAD_PATH}",
        data=m,
        headers={"Content-Type": m.content_type}
    )

    print_result("Multipart single file", r)
    assert_ok(r)


# ---------- MULTIPART (múltiplos arquivos) ----------

def test_multipart_multiple_files():
    m = MultipartEncoder(
        fields={
            "file1": ("test.txt", open(f"{FILES_DIR}/test.txt", "rb"), "text/plain"),
            "file2": ("test.json", open(f"{FILES_DIR}/test.json", "rb"), "application/json"),
            "file3": ("image.png", open(f"{FILES_DIR}/image.png", "rb"), "image/png")
        }
    )

    r = requests.post(
        f"{BASE_URL}{UPLOAD_PATH}",
        data=m,
        headers={"Content-Type": m.content_type}
    )

    print_result("Multipart multiple files", r)
    assert_ok(r)


# ---------- MULTIPART CHUNKED ----------

def test_multipart_chunked():
    path = os.path.join(FILES_DIR, "test.bin")
    size = os.path.getsize(path)

    m = MultipartEncoder(
        fields={
            "file": ("test.bin", open(path, "rb"), "application/octet-stream")
        }
    )

    stream = StreamingIterator(size, m)

    r = requests.post(
        f"{BASE_URL}{UPLOAD_PATH}",
        data=stream,
        headers={
            "Content-Type": m.content_type,
            "Transfer-Encoding": "chunked"
        }
    )

    print_result("Multipart chunked", r)
    assert_ok(r)


# ---------- DELETE (cria antes) ----------

def test_delete_with_setup():
    data = b"delete me"

    # cria arquivo
    r_create = requests.post(
        f"{BASE_URL}{UPLOAD_PATH}",
        data=data,
        headers={
            "Content-Type": "text/plain",
            "Content-Length": str(len(data))
        }
    )

    print_result("Setup file for DELETE", r_create)

    # deleta
    r_delete = requests.delete(f"{BASE_URL}{DELETE_PATH}")
    print_result("DELETE file", r_delete)
    assert_ok(r_delete)


# ---------- CGI ----------

def test_cgi_get():
    r = requests.get(
        CGI_URL,
        params={"name": "MeuNome", "lang": "cpp98"}
    )
    print_result("CGI GET", r)
    assert_ok(r)


def test_cgi_post():
    r = requests.post(
        CGI_URL,
        data={"msg": "hello cgi"}
    )
    print_result("CGI POST", r)
    assert_ok(r)


def test_cgi_chunked():
    def gen():
        yield b"name=meuNome&"
        time.sleep(0.1)
        yield b"project=webserv"

    r = requests.post(
        CGI_URL,
        data=gen(),
        headers={
            "Content-Type": "application/x-www-form-urlencoded",
            "Transfer-Encoding": "chunked"
        }
    )

    print_result("CGI POST chunked", r)
    assert_ok(r)


# ---------- RUNNER ----------

if __name__ == "__main__":
    test_get()

    test_post_raw_text()
    test_post_raw_json()
    test_post_raw_binary()
    # test_post_raw_chunked()

    test_multipart_single_file()
    test_multipart_multiple_files()
    # test_multipart_chunked()

    # test_delete_with_setup()

    # test_cgi_get()
    # test_cgi_post()
    # test_cgi_chunked()
