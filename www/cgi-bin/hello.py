#!/usr/bin/env python3
import cgi
import cgitb
cgitb.enable()

print("Content-Type: text/html\r\n\r\n")
print("<html><body>")
print("<h1>Sucesso: CGI Rodou!</h1>")

form = cgi.FieldStorage()
name = form.getvalue('name', 'Visitante')
data = form.getvalue('data', 'Nenhum dado POST')

print(f"<p>Metodo GET (Nome): <b>{name}</b></p>")
print(f"<p>Metodo POST (Dados): <b>{data}</b></p>")
print("</body></html>")