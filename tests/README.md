Defina essas variáveis no terminal antes de rodar os comandos, assim você só copia e cola:
```bash
H="localhost"
P="8080"
```

# ✅ Testes de Sucesso (Esperado: 200, 201, 204)
1. O Clássico (GET Root) O teste mais básico para ver se o servidor respira.
```bash
printf "GET / HTTP/1.1\r\nHost: $H\r\nConnection: close\r\n\r\n" | nc $H $P
```

2. GET com Query String O servidor deve ser capaz de ignorar ou processar o que vem depois do ?.
```bash
printf "GET /cgi-bin/test.py?busca=teste&id=1 HTTP/1.1\r\nHost: $H\r\nConnection: close\r\n\r\n" | nc $H $P
```

3. POST Pequeno (Simulando JSON)
```bash
printf "POST /upload HTTP/1.1\r\nHost: $H\r\nContent-Type: application/json\r\nContent-Length: 2\r\n\r\n{}" | nc $H $P
```

4. DELETE de Arquivo Testa se o método DELETE é reconhecido.
```bash
printf "DELETE /upload/imagem.png HTTP/1.1\r\nHost: $H\r\nConnection: close\r\n\r\n" | nc $H $P
```

5. Chunked Transfer Encoding (Avançado) Se seu servidor suporta chunked, isso deve funcionar. Envia "Wiki" (4 bytes) e depois "pedia" (5 bytes).
```bash
printf "POST /upload HTTP/1.1\r\nHost: $H\r\nTransfer-Encoding: chunked\r\n\r\n4\r\nWiki\r\n5\r\npedia\r\n0\r\n\r\n" | nc $H $P
```

# ❌ Testes de Erro / Falha (Esperado: 4xx ou 5xx)

6. Versão HTTP Inexistente (Seu erro anterior) Deve retornar 400 Bad Request ou 505 HTTP Version Not Supported.
```bash
printf "GET / HTTP/9.9\r\nHost: $H\r\n\r\n" | nc $H $P
```

7. Falta o Header 'Host' (Obrigatório no HTTP/1.1) Deve retornar 400 Bad Request. Servidores 1.1 estritos não aceitam requisição sem Host.
```bash
printf "GET / HTTP/1.1\r\nConnection: close\r\n\r\n" | nc $H $P
```

8. Método Desconhecido Deve retornar 501 Not Implemented.
```bash
printf "CHURRASCO / HTTP/1.1\r\nHost: $H\r\n\r\n" | nc $H $P
```

9. Header Malformado (Sem dois pontos) Deve retornar 400 Bad Request. O parser deve detectar que User-Agent-Mozilla não tem a separação : .
```bash
printf "GET / HTTP/1.1\r\nHost: $H\r\nUser-Agent-Mozilla\r\n\r\n" | nc $H $P
```

10. POST sem Content-Length Deve retornar 400 Bad Request ou 411 Length Required. Se o servidor não sabe o tamanho do corpo e não é chunked, ele deve rejeitar.
```bash
printf "POST /upload HTTP/1.1\r\nHost: $H\r\n\r\nCorpoSemTamanho" | nc $H $P
```

11. Tentativa de Directory Traversal (Segurança) Tentar sair da pasta raiz. O servidor deve bloquear (403 ou 404) e jamais entregar o /etc/passwd.
```bash
printf "GET ../../../../../etc/passwd HTTP/1.1\r\nHost: $H\r\n\r\n" | nc $H $P
```

12. Teste post simples, tabém podendo ser get, o script cgi mostra o metodo utilizado
```bash
  curl -i -X POST http://localhost:8080/test.py
```
13. Script CGI em python com uma exceção lançada (Teste de falha do CGI)
```bash
  curl -i -X POST http://localhost:8080/cgi-bin/error.py
```
14. Script CGI com while infitnito (Servidor deve responder com timeout)
```bash
  curl -i -X POST http://localhost:8080/cgi-bin/infinite.py
```
15. Recuperar algo enviado pelo post
```bash
    curl -i http://localhost:8080/files/nome do arquivo
```
  