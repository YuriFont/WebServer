import socket
import time
from pathlib import Path
from utils.ensure_file_exist import ensure_test_file

# arquivo de 10MB enviado para o servidor em chunked(pedaços, não sabemos o content-length), um arquivo .txt de 10MB será criado na pasta /uploads, o envio desse e feito com intervalo de tempo entre os pedaços enviados
BASE_DIR = Path(__file__).resolve().parent.parent  # tests/
FILES_DIR = BASE_DIR / "files"
def test_post_raw_existing_txt_10mb_chunked_split_crlf(server_addr):
    host, port = server_addr
    # 1. Preparação do arquivo
    file_path = (FILES_DIR / "text_10mb.txt")
    ensure_test_file(file_path, size_mb=10, pattern=b"X")

    chunk_size = 8192 
    
    # 2. Conectar manualmente (para termos controle do send)
    # Assumindo localhost e porta 8080 (ajuste conforme seu servidor)
    # host = "localhost"
    # port = 8080 
    
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.connect((host, port))

    # 3. Enviar Cabeçalhos
    header = (
        b"POST /upload/ HTTP/1.1\r\n"
        b"Host: localhost\r\n"
        b"Connection: close\r\n"
        b"Transfer-Encoding: chunked\r\n"
        b"Content-Type: text/plain\r\n\r\n"
    )
    s.sendall(header)

    # 4. Enviar Corpo com "Maldade" (Splitting CRLF)
    with file_path.open("rb") as f:
        while True:
            part = f.read(chunk_size)
            if not part:
                break

            size_hex = hex(len(part))[2:].encode()
            
            # AQUI ESTÁ A MÁGICA:
            # Em vez de enviar tudo junto, enviamos fragmentado
            
            # 1. Envia o tamanho e o \r
            s.send(size_hex + b"\r")
            time.sleep(0.002) # Pequena pausa força o flush do pacote TCP
            
            # 2. Envia o \n que completa o tamanho
            s.send(b"\n")
            time.sleep(0.002)

            # 3. Envia o conteúdo
            s.send(part)
            
            # 4. Envia o \r final do bloco
            s.send(b"\r")
            time.sleep(0.002)

            # 5. Envia o \n final do bloco
            s.send(b"\n")

    # 5. Chunk final (0\r\n\r\n) também fragmentado
    s.send(b"0\r")
    time.sleep(0.002)
    s.send(b"\n\r\n")     # Testando uma combinação diferente aqui
    time.sleep(0.002)
    # s.send(b"\n")

    # 6. Ler resposta
    response = b""
    while True:
        data = s.recv(4096)
        if not data:
            break
        response += data
    
    s.close()

    # Validação
    print(f"Resposta recebida:\n{response.decode(errors='ignore')}")
    assert b"200 OK" in response or b"201" in response