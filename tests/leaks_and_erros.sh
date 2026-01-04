#!/bin/bash

SERVER_BIN=./bin/webserv
CONFIG_FILE=./config/default.conf
SERVER_URL=http://localhost:8080
BIG_FILE=/arquivo_grande.html
LIST_URLS=./tests/urls.txt

echo "========================================"
echo " Webserv - Testes automáticos + Sanidade "
echo "========================================"

cd ..

# -----------------------------
# Funções auxiliares
# -----------------------------
count_fds() {
    ls /proc/$1/fd 2>/dev/null | wc -l
}

get_rss_kb() {
    awk '/VmRSS/ { print $2 }' /proc/$1/status 2>/dev/null
}

check_alive() {
    kill -0 $1 2>/dev/null
}

# -----------------------------
# 1. Compilar
# -----------------------------
echo ""
echo "[1] Compilando o projeto..."
make re >/dev/null || exit 1

# -----------------------------
# 2. Iniciar servidor
# -----------------------------
echo ""
echo "[2] Iniciando o servidor..."
$SERVER_BIN $CONFIG_FILE >/dev/null &
SERVER_PID=$!

# Garantir encerramento
cleanup() {
    echo ""
    echo "[FINALIZANDO] Encerrando servidor (PID=$SERVER_PID)"
    kill $SERVER_PID 2>/dev/null
    wait $SERVER_PID 2>/dev/null
}
trap cleanup EXIT

# -----------------------------
# 3. Esperar servidor subir
# -----------------------------
echo "[3] Aguardando servidor iniciar..."
sleep 2

if ! check_alive $SERVER_PID; then
    echo "❌ Servidor morreu ao iniciar"
    exit 1
fi

# -----------------------------
# Medições iniciais
# -----------------------------
FD_BEFORE=$(count_fds $SERVER_PID)
RSS_BEFORE=$(get_rss_kb $SERVER_PID)

echo ""
echo "[INFO] Estado inicial do servidor"
echo "       -> FDs abertos : $FD_BEFORE"
echo "       -> Memória RSS: ${RSS_BEFORE} KB"

# -----------------------------
# 4–9. Testes funcionais
# -----------------------------
echo ""
echo "[4] Teste básico (sanity check)"
siege -c 1 -r 50 "$SERVER_URL" || exit 1

echo ""
echo "[5] Teste de concorrência"
siege -c 50 -r 20 "$SERVER_URL" || exit 1

# echo ""
# echo "[6] Teste de escrita parcial (arquivo grande)"
# siege -c 100 -r 30 "$SERVER_URL$BIG_FILE" || exit 1

echo ""
echo "[7] Teste HTTP/1.0 (sem keep-alive)"
siege -H "Connection: close" -c 50 -r 20 "$SERVER_URL" || exit 1

echo ""
echo "[8] Teste em rotas diferentes"
siege -c 50 -t 30S -f "$LIST_URLS" || exit 1

echo ""
echo "[9] Teste prolongado"
siege -c 100 -t 1M "$SERVER_URL" || exit 1

# -----------------------------
# 10. Verificação de leaks e estabilidade
# -----------------------------
echo ""
echo "[10] Verificação de vazamentos e estabilidade"
sleep 2  # tempo para fechar conexões

if ! check_alive $SERVER_PID; then
    echo "❌ Servidor morreu durante os testes"
    exit 1
fi

FD_AFTER=$(count_fds $SERVER_PID)
RSS_AFTER=$(get_rss_kb $SERVER_PID)

echo "     -> FDs antes : $FD_BEFORE"
echo "     -> FDs depois: $FD_AFTER"
echo "     -> RSS antes : ${RSS_BEFORE} KB"
echo "     -> RSS depois: ${RSS_AFTER} KB"

# --- Validação de FD ---
if [ "$FD_AFTER" -ne "$FD_BEFORE" ]; then
    echo "❌ POSSÍVEL VAZAMENTO DE FILE DESCRIPTORS"
    exit 1
fi

echo "✔ Nenhum vazamento de FD detectado"

# --- Observação de memória (não falha o teste) ---
DELTA_RSS=$((RSS_AFTER - RSS_BEFORE))
echo "     -> Variação RSS: ${DELTA_RSS} KB"

if [ "$DELTA_RSS" -gt 10240 ]; then
    echo "⚠️  Aviso: crescimento de memória acima do esperado"
fi

# -----------------------------
# Final
# -----------------------------
echo ""
echo "========================================"
echo " TODOS OS TESTES PASSARAM COM SUCESSO"
echo "========================================"