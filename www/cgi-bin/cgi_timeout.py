#!/usr/bin/env python3
import os
import sys
import time
import cgi
from datetime import datetime

# Headers HTTP obrigatórios
print("Content-Type: text/html")
print("")

# Lê dados POST se existirem
form = cgi.FieldStorage()
post_data = ""
if form.getvalue('nome'):
    post_data = f"Dados POST recebidos: {form.getvalue('nome')}, {form.getvalue('email')}"

# TESTE DE TIMEOUT - DESCOMENTE UMA DAS OPÇÕES:
TIMEOUT_TEST = True  # ← MUDE PARA False para teste normal

if TIMEOUT_TEST:
    print("<h1>🕐 TESTE DE TIMEOUT ATIVO</h1>")
    print("<p>Simulando processamento lento... Aguarde 40 segundos (timeout = 30s)</p>")
    print("<p><em>Se o timeout estiver funcionando, você verá erro 504 após 30s</em></p>")
    
    # SIMULA CGI LENTO QUE TRAVA (timeout será acionado)
    time.sleep(40)  # 40s > 30s timeout
    
    print("<p>✅ Timeout test concluído!</p>")
else:
    # TESTE NORMAL (rápido)
    print("""
    <!DOCTYPE html>
    <html>
    <head>
        <title>CGI Python WebServ - OK</title>
        <meta charset="UTF-8">
    </head>
    <body>
        <h1>✅ CGI Python funcionando perfeitamente!</h1>
        
        <h2>Variáveis CGI:</h2>
        <ul>
            <li><strong>Método:</strong> {}
            <li><strong>Path:</strong> {}
            <li><strong>Query:</strong> {}
            <li><strong>Content-Length:</strong> {}
            <li><strong>Servidor:</strong> {}</li>
        </ul>
        
        <h3>Formulário POST:</h3>
        <form method="POST">
            <input type="text" name="nome" placeholder="Nome" required>
            <input type="email" name="email" placeholder="Email">
            <button>Testar POST</button>
        </form>
        
        {}
        
        <hr><small>Testado em: {}</small>
    </body>
    </html>
    """.format(
        os.environ.get('REQUEST_METHOD', 'N/A'),
        os.environ.get('PATH_INFO', 'N/A'),
        os.environ.get('QUERY_STRING', 'N/A'),
        os.environ.get('CONTENT_LENGTH', '0'),
        os.environ.get('SERVER_NAME', 'localhost') + ':' + os.environ.get('SERVER_PORT', '8080'),
        post_data,
        datetime.now().strftime("%Y-%m-%d %H:%M:%S")
    ))
