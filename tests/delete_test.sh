#!/bin/bash

SERVER_BINARY=./bin/webserv
CONF_FILE=./config/tests/delete_test.conf
UPLOAD_DIR=./www/uploads
PORT=8081
LOGFILE="tests/logs/delete_test.log"
mkdir -p logs


cd ..

make re >/dev/null || { echo "❌ Falha na compilação"; exit 1; }
touch "$LOGFILE"
$SERVER_BINARY $CONF_FILE > "$LOGFILE" &
PID=$!

sleep 1

exit_program() {
    CODE=$1
    kill $PID
    wait $PID 2>/dev/null
    exit $CODE
}


echo "🧪 Testes do método DELETE"

# 1️⃣ Deleta arquivo existente
echo "→ Criando arquivo para deletar..."
echo "delete me" > $UPLOAD_DIR/test1.txt
STATUS_LINE=$(curl -s -D - -o /dev/null -X DELETE http://localhost:$PORT/upload/test1.txt | head -n 1)

STATUS=$(echo "$STATUS_LINE" | awk '{print $2}')
STATUS_NAME=$(echo "$STATUS_LINE" | cut -d' ' -f3-)

if [ -e "$UPLOAD_DIR/test1.txt" ]; then
  echo "❌ Arquivo não foi deletado"
  rm -rf $UPLOAD_DIR/test1.txt
  exit_program 1
else
  echo "🗑️  Arquivo de teste deletado com successo"
fi

if [ "$STATUS" = "204" ]; then
  echo "✅ DELETE arquivo existente status da resposta ($STATUS): $STATUS_NAME"
else
  echo "❌ DELETE arquivo existente falhou ($STATUS) $STATUS_NAME"
  exit_program 1
fi

sleep 1

# 2️⃣ Tenta deletar arquivo inexistente
STATUS=$(curl -s -o /dev/null -w "%{http_code}" -X DELETE http://localhost:$PORT/upload/notfound.txt)
if [ "$STATUS" = "404" ]; then
  echo "✅ DELETE arquivo inexistente (404)"
else
  echo "❌ DELETE arquivo inexistente falhou ($STATUS)"
  exit_program 1
fi

sleep 1

# 3️⃣ Tenta deletar diretório
mkdir -p $UPLOAD_DIR/dirtest
STATUS=$(curl -s -o /dev/null -w "%{http_code}" -X DELETE http://localhost:$PORT/upload/dirtest)
if [ "$STATUS" = "403" ] || [ "$STATUS" = "409" ]; then
  echo "✅ DELETE diretório não permitido ($STATUS)"
else
  echo "❌ DELETE diretório falhou ($STATUS)"
  exit_program 1
fi
rm -rf $UPLOAD_DIR/dirtest

sleep 1

# 4️⃣ Tenta deletar arquivo sem permissão
mkdir -p $UPLOAD_DIR/dirtest
echo "locked" > $UPLOAD_DIR/dirtest/protected.txt
chmod 400 $UPLOAD_DIR/dirtest
STATUS=$(curl -s -o /dev/null -w "%{http_code}" -X DELETE http://localhost:$PORT/upload/dirtest/protected.txt)
if [ "$STATUS" = "403" ] || [ "$STATUS" = "500" ]; then
  echo "✅ DELETE sem permissão respondeu corretamente ($STATUS)"
else
  echo "❌ DELETE sem permissão falhou ($STATUS)"
  chmod 744 $UPLOAD_DIR/dirtest
  rm -rf $UPLOAD_DIR/dirtest
  exit_program 1
fi

chmod 744 $UPLOAD_DIR/dirtest
rm -rf $UPLOAD_DIR/dirtest

sleep 1

echo "🧹 Teste delete encerrado"
exit_program 0