#!/bin/bash

SERVER_BINARY=./bin/webserv
CONF_FILE=./config/test.conf
PORT=8080
LOGFILE="tests/logs/cgi_test.log"

CGI_PATH="/cgi-bin/test.py"
CGI_PATH_GLOBAL="/www/html/test.py"
CGI_URL="http://localhost:$PORT$CGI_PATH"
CGI_URL_GLOBAL="http://localhost:$PORT$CGI_PATH_GLOBAL"
mkdir -p logs

cd ..

make re >/dev/null || { echo "❌ Falha na compilação"; exit 1; }

$SERVER_BINARY $CONF_FILE > "$LOGFILE" 2>&1 &
PID=$!
sleep 1

exit_program() {
    CODE=$1
    kill $PID
    wait $PID 2>/dev/null
    exit $CODE
}

echo "🧪 Testes de CGI Global (GET)"

# 1️⃣ GET em CGI existente (CGI GLOBAL)
STATUS=$(curl -s -o /dev/null -w "%{http_code}" "$CGI_URL_GLOBAL")
if [ "$STATUS" = "200" ]; then
  echo "✅ GET CGI global ($CGI_PATH_GLOBAL) retornou 200"
else
  echo "❌ GET CGI global falhou ($STATUS)"
  exit_program 1
fi

sleep 1

# 2️⃣ Verifica se o conteúdo vem do CGI (stdout)
BODY=$(curl -s "$CGI_URL")
if echo "$BODY" | grep -q "Python CGI Test"; then
  echo "✅ CGI executado corretamente (conteúdo esperado)"
else
  echo "❌ Resposta não parece ser de um CGI"
  exit_program 1
fi

sleep 1

# 3️⃣ GET em CGI inexistente
STATUS=$(curl -s -o /dev/null -w "%{http_code}" http://localhost:$PORT/cgi-bin/missing.bla)
if [ "$STATUS" = "404" ]; then
  echo "✅ CGI inexistente retornou 404"
else
  echo "❌ CGI inexistente falhou ($STATUS)"
  exit_program 1
fi

exit_program 0
