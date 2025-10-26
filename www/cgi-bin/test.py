#!/usr/bin/env python3

import os
import sys

print("Content-Type: text/html\n")  # header obrigatório + linha em branco

print("<html>")
print("<body>")
print("<h1>Python CGI Test</h1>")

# Mostrar método
print("<p>Method: {}</p>".format(os.environ.get("REQUEST_METHOD", "")))

# Mostrar query string
print("<p>Query string: {}</p>".format(os.environ.get("QUERY_STRING", "")))

# Mostrar POST body (se houver)
if os.environ.get("REQUEST_METHOD", "") == "POST":
    length = int(os.environ.get("CONTENT_LENGTH", 0))
    data = sys.stdin.read(length)
    print("<p>POST data: {}</p>".format(data))

print("</body>")
print("</html>")