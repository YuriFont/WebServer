#!/bin/bash

SERVER_BIN=./bin/webserv
CONFIG_FILE=./config/default.conf
SERVER_URL=http://localhost:8080
BIG_FILE=/arquivo_grande.html

echo "========================================"
echo " Webserv - Testes automáticos com Siege"
echo "========================================"

# 1. Compilar
echo ""
echo "[1] Compilando o projeto..."
make re >/dev/null || exit 1

# 2. Iniciar servidor
echo ""
echo "[2] Iniciando o servidor..."
$SERVER_BIN $CONFIG_FILE >/dev/null &
SERVER_PID=$!

# Garantir que o servidor será finalizado
cleanup() {
    echo ""
    echo "[FINALIZANDO] Encerrando servidor (PID=$SERVER_PID)"
    kill $SERVER_PID 2>/dev/null
    wait $SERVER_PID 2>/dev/null
}
trap cleanup EXIT

# 3. Esperar servidor subir
echo "[3] Aguardando servidor iniciar..."
sleep 2

# 4. Testes
echo ""
echo "[4] Teste básico (sanity check)"
siege -c 1 -r 50 "$SERVER_URL" || exit 1

echo ""
echo "[5] Teste de concorrência"
siege -c 50 -r 20 "$SERVER_URL" || exit 1

echo ""
echo "[6] Teste de escrita parcial (arquivo grande)"
siege -c 100 -r 30 "$SERVER_URL$BIG_FILE" || exit 1

echo ""
echo "[7] Teste com keep-alive"
siege -c 50 -r 20 --keep-alive "$SERVER_URL" || exit 1

echo ""
echo "[8] Teste prolongado"
siege -c 100 -t 1M "$SERVER_URL" || exit 1

echo ""
echo "========================================"
echo " TODOS OS TESTES PASSARAM COM SUCESSO"
echo "========================================"