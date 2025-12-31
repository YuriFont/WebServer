import pytest

@pytest.fixture(scope="session")
def server_addr(pytestconfig):
    host = pytestconfig.getini("webserv_host")
    port = int(pytestconfig.getini("webserv_port"))
    return host, port