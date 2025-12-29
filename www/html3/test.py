#!/usr/bin/env python3

import os
import sys

# Cabeçalho CGI — DEVE usar CRLF e duas quebras
sys.stdout.write("Content-Type: text/html\r\n\r\n")

sys.stdout.write("<html>\n")
sys.stdout.write("<body>\n")
sys.stdout.write("<h1>Python CGI Test</h1>\n")

# Mostrar método
method = os.environ.get("REQUEST_METHOD", "")
sys.stdout.write(f"<p>Method: {method}</p>\n")

# Mostrar query string
query = os.environ.get("QUERY_STRING", "")
sys.stdout.write(f"<p>Query string: {query}</p>\n")

# Mostrar POST body (se houver)
if method == "POST":
    length = int(os.environ.get("CONTENT_LENGTH", 0))
    data = sys.stdin.read(length)
    sys.stdout.write(f"<p>POST data: {data}</p>\n")

sys.stdout.write("</body>\n")
sys.stdout.write("</html>\n")