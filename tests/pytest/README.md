# Webserv – Testes Automatizados

Os testes validam o comportamento real do servidor através de sockets TCP, sem mocks, cobrindo parsing HTTP, status codes, CGI, upload de arquivos, multipart e transferência em chunked encoding.

## Pré-requisitos

- Necessário ter a rota /upload com post habilitado
- Um arquivo test.py acessível na rota /cgi-bin/test.py aceitando corpo e o devolvendo com status 200
- Baixe um video .mp4 qualquer e coloque dentro da pasta files/ e renomeie o video para video.mp4
- Arquivos .txt de 10mb e 100mb serão criados dentro da pasta files/
- O teste de max_body_size o tamanho do arquivo para envio está setado para 150mb está dentro de `codes/test_status_codes.py`

**Pode ser alterado via CLI ao executar**
- `http_client.py` está configurado como padrão o envio de requisições para 127.0.0.1 na porta 8080
- `http.py` roda por padrão no localhost na porta 8080
- `test_client_disconnect.py` está por padrão na 127.0.0.1 na porta 8080
- `test_idle_client.py` está por padrão na 127.0.0.1 na porta 8080


## 📁 Estrutura dos Testes
```
tests./
├── README.md
├── pytest/
│   ├── cgi/                   # Testes relacionados a CGI
│   ├── codes/                 # Testes de status codes HTTP
│   ├── get/                   # Testes GET (arquivos estáticos e CGI)
│   ├── post/                  # Testes POST (raw, form, multipart, chunked)
│   ├── files/                 # Arquivos usados nos testes (bin, txt, png, pdf, mp4)
│   ├── utils/                 # Funções auxiliares (socket HTTP, geração de arquivos)
│   └── conf/
│       └── test.conf              # Configuração dos testes (Sem uso)
```


## ▶️ Como executar os testes

### Instalar Python e Pip

```bash
sudo apt update
sudo apt install python3 python3-pip
```

### Instalar as Dependências

```bash
pip install pytest pytest-xdist requests
```

### Arquivo `pytest.ini`

No arquivo `conftest.py` por padrão e configurado as seguinte interface `127.0.0.1` e a seguinte porta `8080`, mas e possivel mudar ao executar o pytest, por exemplo:

```bash
pytest --webserv-host <interface> --webserv-port <porta>
```


### 1️⃣ Subir o servidor manualmente

Os testes assumem que o servidor já está rodando:
```bash
./webserv ./conf/default.conf
```

### 2️⃣ Executar o pytest

```bash
pytest -v tests/
```
ou se não for o padrão de interface e porta configurado, substitua localhost e 9090 pelas suas configurações
```bash
pytest -v tests/ --webserv-host localhost --webserv-port 9090
```

## 🔧 Utilitários principais

`utils/http_client.py`
Envio de requisições HTTP cruas, diretamente por socket:
```python
send_raw_http(request: bytes) -> bytes
```
Usado para testar:
- parsing HTTP
- chunked encoding
- fragmentação de pacotes
- comportamento real de rede

`utils/http.py`
Interface simples via requests para testes básicos:
```python
get(path)
post(path)
delete(path)
```
Usado principalmente em testes de status codes simples.

`utils/ensure_file_exist.py`
Criação automática de arquivos grandes (10MB, 100MB) para testes:
- evita versionar arquivos pesados
- garante reprodutibilidade
- usado em testes de chunked upload

## ✅ Cobertura dos Testes

### 🔹 Status Codes (codes/)

- **200 OK** – requisições válidas
- **404 Not Found** – recurso inexistente
- **405 Method Not Allowed** – método não permitido
- **400 Bad Request** – request malformado
- **413 Payload Too Large** – body acima do limite

Esses testes validam a correção do parser HTTP e das respostas do servidor.

### 🔹 GET (get/)

- GET de arquivo estático (index.html)
- GET em CGI com query string
Valida:
- roteamento (Quem vai atender a resposta)
- leitura de arquivos
- execução de CGI

### 🔹 CGI (cgi/)

- POST em CGI com Content-Length
- POST em CGI com Transfer-Encoding: chunked
Valida:
- execução correta do CGI
- passagem do corpo da requisição
- suporte a chunked em CGI

### 🔹 POST (post/)

Cobertura extensa de uploads:
- application/x-www-form-urlencoded
- multipart/form-data
- arquivos binários
- múltiplos arquivos no mesmo request
- uploads em chunked encoding
- uploads grandes (10MB / 100MB)
- fragmentação intencional de pacotes TCP (CRLF split)
Esses testes validam:
- parsing incremental
- bufferização correta (Nem sempre toda a requisição e recebida em um unico recv)
- robustez do servidor contra fragmentação (Envio do cliente para o servidor pode ser feito em pedaços)

## 🧨 Testes de fragmentação (caso extremo)

O teste:
```
test_16_post_chunked_txt_10MB_fragmented.py
```
Envia dados quebrando propositalmente o protocolo em múltiplos send(), para validar que:
- o servidor lida corretamente com leitura parcial
- não assume alinhamento de CRLF
- não depende de pacotes completos
Esse teste simula comportamento real de rede.

## ⚠️ Testes que exigem configuração manual

### test_idle_client_timeout (**RESOLVIDO**)

O teste `tests/codes/test_idle_client_timeout.py` **não utiliza o endereço padrão dinamico**.

Por padrão, ele **exige que o IP e a porta sejam ajustados manualmente no código**
para apontar para o servidor correto em execução.

Isso é necessário porque:
- o teste mantém a conexão aberta intencionalmente (cliente ocioso);
- o timeout é validado no nível de socket;

Antes de executar este teste, ajuste no arquivo:

```python
host = "127.0.0.1"
port = 8080
```

### test_403_forbidden (permissões de arquivo)

O teste `codes/test_status_codes.py -> test_403_forbidden()` **não é executado automaticamente** por padrão.

Este teste exige:
- criação manual do arquivo `secret.html` dentro de `www/html`
- alteração explícita das permissões do arquivo (`chmod 000`)

O objetivo do teste é validar se o servidor retorna **403 Forbidden**
quando um recurso existe, mas **não possui permissões de leitura**.

Por segurança e portabilidade, o teste foi comentado, pois:
- modifica permissões reais do filesystem;
- pode falhar em ambientes sem suporte a `chmod` (Windows, WSL restrito);
- pode deixar arquivos inacessíveis se a execução for interrompida.

### Teste `codes/test_status_codes.py/test_413_payload_too_large`

Este teste valida o comportamento do servidor quando o corpo da requisição
ultrapassa o limite máximo configurado no arquivo `.conf`.

⚠️ **Pré-requisito**  
É necessário definir explicitamente um valor de `max_body_size` (ou equivalente)
no arquivo de configuração utilizado pelo servidor durante os testes.

