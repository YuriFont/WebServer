#!/bin/bash

# CORES PARA SAÍDA
GREEN='\033[0;32m'
RED='\033[0;31m'
YELLOW='\033[1;33m'
RESET='\033[0m'

# CONFIGURAÇÃO
HOST="127.0.0.1"
PORT="8080" # Mude para a porta do seu servidor
SERVER_BIN="./bin/webserv" # Opcional: Se quiser checar se o processo morre

echo -e "${YELLOW}=== INICIANDO TESTES DE I/O E CONFORMIDADE DE SOCKETS ===${RESET}"
echo "Certifique-se de que seu servidor está rodando em $HOST:$PORT"
echo ""

# ---------------------------------------------------------
# 1. TESTE DE CONEXÃO E DESCONEXÃO ABRUPTA (READ == 0)
# ---------------------------------------------------------
echo -e "${YELLOW}[1] Testando Cliente que desconecta no meio do envio...${RESET}"
echo "   -> Enviando metade de um header e fechando socket..."

python3 -c "
import socket
import time
try:
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.connect(('$HOST', int('$PORT')))
    s.send(b'GET / HTTP/1.1\r\nHost: loc') # Envia incompleto
    time.sleep(1) # Espera o servidor processar
    s.close() # Fecha na cara do servidor (deve gerar read() == 0)
    print('${GREEN}   -> Socket fechado pelo cliente.${RESET}')
except Exception as e:
    print(f'${RED}   -> Erro ao conectar: {e}${RESET}')
"

echo -e "${YELLOW}   -> VERIFIQUE NO SERVIDOR:${RESET} Ele deve mostrar 'Connection closed' e NÃO entrar em loop infinito."
echo "   Pressione ENTER para continuar..."
read

# ---------------------------------------------------------
# 2. TESTE DE LEITURA LENTA (SLOW CLIENT)
# ---------------------------------------------------------
echo -e "${YELLOW}[2] Testando Cliente Lento (Slow Read)...${RESET}"
echo "   -> Enviando request byte a byte..."

python3 -c "
import socket
import time
try:
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.connect(('$HOST', int('$PORT')))
    msg = b'GET / HTTP/1.1\r\nHost: localhost\r\n\r\n'
    for byte in msg:
        s.send(bytes([byte]))
        time.sleep(0.05) # 50ms entre bytes
    data = s.recv(1024)
    print(f'${GREEN}   -> Resposta recebida ({len(data)} bytes).${RESET}')
    s.close()
except Exception as e:
    print(f'${RED}   -> Erro: {e}${RESET}')
"
echo -e "${YELLOW}   -> SUCESSO SE:${RESET} O servidor respondeu 200 OK e não travou outros clientes."
echo ""

# ---------------------------------------------------------
# 3. TESTE DE ESCRITA GRANDE (WRITE CHUNKING)
# ---------------------------------------------------------
echo -e "${YELLOW}[3] Testando Upload de Arquivo Grande (Stress no write/read)...${RESET}"

# Cria arquivo de 5MB
dd if=/dev/urandom of=temp_large_file.bin bs=1M count=5 2>/dev/null

# Tenta fazer upload (POST)
# Assumindo que você tem uma rota que aceita POST. Se não, mude para GET de arquivo grande.
HTTP_CODE=$(curl -s -o /dev/null -w "%{http_code}" -X POST --data-binary @temp_large_file.bin http://$HOST:$PORT/upload)

if [ "$HTTP_CODE" -eq 200 ] || [ "$HTTP_CODE" -eq 201 ]; then
    echo -e "${GREEN}   -> Upload concluído. Código HTTP: $HTTP_CODE${RESET}"
else
    echo -e "${RED}   -> Falha no upload. Código HTTP: $HTTP_CODE${RESET}"
fi

rm temp_large_file.bin
echo -e "${YELLOW}   -> VERIFIQUE NO SERVIDOR:${RESET} Não deve haver erro de 'Broken Pipe' ou travamento."
echo ""

# ---------------------------------------------------------
# 4. VERIFICAÇÃO DE FILE DESCRIPTORS (LEAK TEST)
# ---------------------------------------------------------
echo -e "${YELLOW}[4] Verificando Vazamento de FDs...${RESET}"
echo "   -> Vamos fazer 50 conexões rápidas e checar se os FDs abertos aumentam permanentemente."

# Pega o PID do webserv (assumindo que o nome do processo é 'webserv')
SERVER_PID=$(pgrep -f webserv | grep -v "test_io.sh" | head -n 1)

if [ -z "$SERVER_PID" ]; then
    echo -e "${RED}   -> Processo 'webserv' não encontrado. Pulei o teste de FDs.${RESET}"
else
    FD_BEFORE=$(ls /proc/$SERVER_PID/fd/ | wc -l)
    echo "   -> FDs abertos antes: $FD_BEFORE"

    # Bombardeio com nc (netcat)
    for i in {1..50}; do
        echo "GET / HTTP/1.1\r\nHost: localhost\r\n\r\n" | nc -w 1 $HOST $PORT > /dev/null 2>&1 &
    done
    wait

    sleep 2 # Espera conexões fecharem
    FD_AFTER=$(ls /proc/$SERVER_PID/fd/ | wc -l)
    echo "   -> FDs abertos depois: $FD_AFTER"

    if [ "$FD_AFTER" -le "$(($FD_BEFORE + 2))" ]; then
        echo -e "${GREEN}   -> SUCESSO: Parece que não há vazamento de FDs.${RESET}"
    else
        echo -e "${RED}   -> PERIGO: FDs aumentaram! Verifique se está chamando close() em todos os casos de erro.${RESET}"
    fi
fi

# ---------------------------------------------------------
# 5. TESTE DE BROKEN PIPE (ERRO DE ESCRITA)
# ---------------------------------------------------------
echo -e "${YELLOW}[5] Testando Broken Pipe (Cliente fecha enquanto servidor escreve)...${RESET}"

python3 -c "
import socket
import time
import sys

try:
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.connect(('$HOST', int('$PORT')))
    
    # Pede algo grande (assumindo que '/' retorna uma página HTML)
    s.send(b'GET / HTTP/1.1\r\nHost: localhost\r\n\r\n')
    
    # Lê só o comecinho e fecha abruptamente
    data = s.recv(10)
    print(f'${GREEN}   -> Recebi {len(data)} bytes e fechei a porta na cara do servidor.${RESET}')
    s.close() 
    
    # O servidor deve tentar escrever o resto, falhar (retorno -1 ou SIGPIPE)
    # e fechar o FD sem derrubar o processo.
    
except Exception as e:
    print(f'${RED}   -> Erro no script python: {e}${RESET}')
"

echo -e "${YELLOW}   -> VERIFIQUE NO SERVIDOR:${RESET}"
echo "      1. O servidor NÃO pode ter crashado (Segfault/SIGPIPE)."
echo "      2. Deve aparecer log de erro no envio ou desconexão."
echo "      3. O FD deve ser fechado."
echo ""

echo ""
echo -e "${YELLOW}=== FIM DOS TESTES ===${RESET}"
