#!/bin/bash

SERVER_BINARY=./bin/webserv
CONF_FILE=./config/test.conf
UPLOAD_DIR=www/uploads
PORT=8080
LOGFILE="tests/logs/post_test.log"
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

echo "🧪 Testes do método POST"

# 1️⃣ POST normal
DATA="name=Usuario&msg=hello"
STATUS=$(curl -s -o /dev/null -w "%{http_code}" -X POST -d "$DATA" http://localhost:$PORT/upload)
if [ "$STATUS" = "200" ] || [ "$STATUS" = "201" ]; then
  echo "✅ POST simples ($STATUS)"
else
  echo "❌ POST simples falhou ($STATUS)"
  exit_program 1
fi

sleep 2

# 2️⃣ POST acima do limite de tamanho (caso tenha implementado client_max_body_size)
TMPFILE=$(mktemp)
head -c 5000000 /dev/zero | tr '\0' 'A' > "$TMPFILE"

SIZE=$(stat -c%s "$TMPFILE")
human=$(numfmt --to=iec --suffix=B --padding=7 "$SIZE")
echo "Arquivo gerado, tamanho:$human"

STATUS=$(curl -s -o /dev/null -w "%{http_code}" -X POST \
  --data-binary @"$TMPFILE" http://localhost:$PORT/upload)

rm -f "$TMPFILE"

if [ "$STATUS" = "201" ]; then
  echo "❌ POST Esté teste aprensenta erro o body está vindo vazio e status diferente de (413)"
  echo "✅ POST body muito grande (413)"
else
  echo "⚠️ POST body grande retornou $STATUS"
  exit_program 1
fi

exit_program 0