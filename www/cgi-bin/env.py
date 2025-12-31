#!/usr/bin/python3
import os

# 1. Cabeçalhos HTTP (Necessário pular uma linha depois)
print("Content-Type: text/html\r\n\r\n")

# 2. Corpo do HTML
print("<!DOCTYPE html>")
print("<html lang='en'>")
print("<head>")
print("<meta charset='UTF-8'>")
print("<title>CGI Environment Dump</title>")
print("<style>")
print("body { font-family: sans-serif; padding: 20px; background: #f4f4f4; }")
print("table { width: 100%; border-collapse: collapse; background: white; box-shadow: 0 2px 5px rgba(0,0,0,0.1); }")
print("th, td { border: 1px solid #ddd; padding: 10px; text-align: left; }")
print("th { background-color: #007bff; color: white; }")
print("tr:nth-child(even) { background-color: #f9f9f9; }")
print("</style>")
print("</head>")
print("<body>")

print("<h1>CGI Environment Variables</h1>")
print("<p>Vars received from the WebServer:</p>")

# 3. Gerar a tabela com as variáveis
print("<table>")
print("<tr><th>Variable Name</th><th>Value</th></tr>")

# Loop para pegar todas as variáveis de ambiente
for key, value in os.environ.items():
    print(f"<tr><td>{key}</td><td>{value}</td></tr>")

print("</table>")
print("</body></html>")