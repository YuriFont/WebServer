#!/bin/bash

SERVER_BINARY=./bin/webserv
CONF_FILE=./config/tests/redirect_test.conf
PORT=8084
LOGFILE="tests/logs/redirect_test.log"
mkdir -p tests/logs

cd ..

make re >/dev/null || { echo "❌ Falha na compilação"; exit 1; }

$SERVER_BINARY "$CONF_FILE" > "$LOGFILE" &
PID=$!
sleep 1

exit_program() {
    CODE=$1
    if [ ! -z "$PID" ] && ps -p $PID > /dev/null; then
        kill $PID
        wait $PID 2>/dev/null
    fi
    exit $CODE
}

echo "🔄 Teste de Redirecionamento Externo (302) Simplificado"

# --- Variáveis de Teste ---
# Teste Externo: /oldpage -> https://google.com/ (302)
REDIRECT_EXTERNAL_SOURCE="/oldpage"
REDIRECT_EXTERNAL_DESTINATION="https://google.com/"
EXPECTED_STATUS_EXTERNAL="302"

# Função de Teste Centralizada
# $1: URL de origem
# $2: Status esperado (302)
# $3: URL de destino esperada
# $4: Nome do teste
run_redirect_test() {
    local SOURCE=$1
    local EXPECTED_STATUS=$2
    local EXPECTED_DESTINATION=$3
    local TEST_NAME=$4

    echo ""
    echo "--- $TEST_NAME ($EXPECTED_STATUS) ---"

    # Faz a requisição e captura o status e o cabeçalho Location
    INFO=$(curl -s -o /dev/null -w "STATUS:%{http_code}\nURL:%{redirect_url}" http://localhost:$PORT$SOURCE)
    
    # Extrai o status
    STATUS=$(echo "$INFO" | grep "STATUS:" | cut -d ':' -f 2 | tr -d '\n' | xargs)
    
    # Extrai a URL
    URL=$(echo "$INFO" | grep "URL:" | cut -d ':' -f 2- | tr -d '\n' | xargs)

    if [ "$STATUS" = "$EXPECTED_STATUS" ] && [ "$URL" = "$EXPECTED_DESTINATION" ]; then
        echo "✅ Sucesso. Status: $STATUS."
        echo "    -> Redirecionado para: $URL"
    else
        echo "❌ Falha. Status esperado: $EXPECTED_STATUS. Recebido: $STATUS"
        echo "    -> URL Esperada: '$EXPECTED_DESTINATION'"
        echo "    -> URL Recebida: '$URL'"
        exit_program 1
    fi
}

# 1. Teste de Redirecionamento Externo (302)
run_redirect_test \
    "$REDIRECT_EXTERNAL_SOURCE" \
    "$EXPECTED_STATUS_EXTERNAL" \
    "$REDIRECT_EXTERNAL_DESTINATION" \
    "1. Externo: $REDIRECT_EXTERNAL_SOURCE -> $REDIRECT_EXTERNAL_DESTINATION"


# Limpeza e Sucesso
echo ""
echo "✨ Teste de redirecionamento concluído com sucesso!"
exit_program 0