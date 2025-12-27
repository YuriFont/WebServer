#!/bin/bash

# --- Configurações ---
SERVER_BINARY=./bin/webserv
CONF_FILE=./config/tests/cgi_permission_test.conf
PORT=8085
LOGFILE="tests/logs/cgi_test.log"

# Como você roda de dentro de 'tests/', o root fica "ao lado" em ../www/cgi-bin
# Mas como logo abaixo damos 'cd ..', o caminho relativo passa a ser a partir da raiz:
CGI_ROOT="./www/cgi-bin" 

mkdir -p logs
mkdir -p "../www/cgi-bin" # Garante a pasta cgi antes de subir o nível

# Volta para a raiz do projeto (assumindo que o script rodou de /tests)
cd ..

# --- Compilação ---
echo "🔨 Compilando..."
make re >/dev/null || { echo "❌ Falha na compilação"; exit 1; }

# --- Setup dos Arquivos de Teste ---
echo "📝 Criando arquivos temporários..."

# 1. Arquivo Válido (Usando env python3 para compatibilidade universal)
echo "#!/usr/bin/env python3
print('Content-Type: text/plain\r\n\r\nCGI OK')" > "$CGI_ROOT/test_ok.py"
chmod +x "$CGI_ROOT/test_ok.py"

# 2. Arquivo Sem Permissão
touch "$CGI_ROOT/test_forbidden.py"
chmod 000 "$CGI_ROOT/test_forbidden.py"

# --- Iniciar Servidor ---
$SERVER_BINARY $CONF_FILE > "$LOGFILE" &
PID=$!
echo "🚀 Servidor iniciado no PID $PID (Porta $PORT)"
sleep 2

# --- Função de Saída (Cleanup) ---
exit_program() {
    CODE=$1
    echo "🧹 Limpando ambiente..."
    kill $PID
    wait $PID 2>/dev/null
    
    # Remove arquivos criados
    rm -f "$CGI_ROOT/test_ok.py" "$CGI_ROOT/test_forbidden.py"
    
    exit $CODE
}

echo "🧪 Testes do CGI e Validações"

# 1️⃣ CGI Válido (200)
STATUS=$(curl -s -o /dev/null -w "%{http_code}" http://localhost:$PORT/cgi-bin/test_ok.py)
if [ "$STATUS" = "200" ]; then
  echo "✅ CGI Execução OK (200)"
else
  echo "❌ CGI Execução falhou. Esperado 200, recebeu $STATUS"
  exit_program 1
fi

sleep 0.5

# 2️⃣ CGI Inexistente (404)
STATUS=$(curl -s -o /dev/null -w "%{http_code}" http://localhost:$PORT/cgi-bin/arquivo_fantasma.py)
if [ "$STATUS" = "404" ]; then
  echo "✅ CGI Inexistente detectado (404)"
else
  echo "❌ Validação de existência falhou. Esperado 404, recebeu $STATUS"
  exit_program 1
fi

sleep 0.5

# 3️⃣ CGI Sem Permissão (403)
STATUS=$(curl -s -o /dev/null -w "%{http_code}" http://localhost:$PORT/cgi-bin/test_forbidden.py)
if [ "$STATUS" = "403" ]; then
  echo "✅ CGI Permissão negada detectada (403)"
else
  echo "❌ Validação de permissão falhou. Esperado 403, recebeu $STATUS"
  exit_program 1
fi

sleep 0.5

# 4️⃣ Path Traversal (403)
STATUS=$(curl -s -o /dev/null -w "%{http_code}" --path-as-is http://localhost:$PORT/cgi-bin/../../Makefile)
if [ "$STATUS" = "403" ]; then
  echo "✅ Path Traversal bloqueado (403)"
else
  echo "❌ Path Traversal falhou. Esperado 403, recebeu $STATUS"
  exit_program 1
fi

# Sucesso
echo "🎉 Todos os testes de CGI passaram!"
exit_program 0