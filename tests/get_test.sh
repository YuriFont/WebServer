#!/bin/bash

SERVER_BINARY=./bin/webserv
CONF_FILE=./config/test.conf
PORT=8080
LOGFILE="tests/logs/get_test.log"
mkdir -p logs

cd ..

make re >/dev/null || { echo "❌ Falha na compilação"; exit 1; }

$SERVER_BINARY $CONF_FILE > "$LOGFILE" &
PID=$!
sleep 1

exit_program() {
    CODE=$1
    kill $PID
    wait $PID 2>/dev/null
    exit $CODE
}

echo "🧪 Testes do método GET"

# 1️⃣ GET de arquivo existente
STATUS=$(curl -s -o /dev/null -w "%{http_code}" http://localhost:$PORT/index.html)
if [ "$STATUS" = "200" ]; then
  echo "✅ GET index.html (200)"
else
  echo "❌ GET index.html falhou ($STATUS)"
  exit_program 1
fi

sleep 1

# 2️⃣ GET de arquivo inexistente
STATUS=$(curl -s -o /dev/null -w "%{http_code}" http://localhost:$PORT/missing.html)
if [ "$STATUS" = "404" ]; then
  echo "✅ GET 404 (404)"
else
  echo "❌ GET 404 falhou ($STATUS)"
  exit_program 1
fi

sleep 1

# 3️⃣ GET de diretório com index
STATUS=$(curl -s -o /dev/null -w "%{http_code}" http://localhost:$PORT/)
if [ "$STATUS" = "200" ]; then
  echo "✅ GET diretório (200)"
else
  echo "❌ GET diretório falhou ($STATUS)"
  exit_program 1
fi

exit_program 0
