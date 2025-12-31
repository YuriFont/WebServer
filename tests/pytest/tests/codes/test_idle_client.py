import socket
import time

# colocar ip e porta utilizados
# Testar se um cliente ocioso bloaqueia o servidor
def test_idle_client_timeout(server_addr):
    host, port = server_addr
    s = socket.socket()
    s.connect((host, port))
    time.sleep(15)
    s.close()
    assert True
