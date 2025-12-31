# tests./conftest.py
import subprocess
import time
import pytest

# Iniciar o servidor automaticamente
# @pytest.fixture(scope="session", autouse=True)
# def webserv_server():
#     proc = subprocess.Popen(
#         ["./webserv", "configs/test.conf"],
#         stdout=subprocess.DEVNULL,
#         stderr=subprocess.DEVNULL
#     )

#     time.sleep(1)  # tempo para subir

#     yield

#     proc.terminate()
#     proc.wait()

def pytest_addoption(parser):
    parser.addoption(
        "--webserv-host",
        action="store",
        default="127.0.0.1",
        help="Webserv host"
    )
    parser.addoption(
        "--webserv-port",
        action="store",
        default="8080",
        help="Webserv port"
    )


@pytest.fixture
def server_addr(pytestconfig):
    host = pytestconfig.getoption("--webserv-host")
    port = int(pytestconfig.getoption("--webserv-port"))
    return host, port