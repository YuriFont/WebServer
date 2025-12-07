#!/usr/bin/env python3
import os
import sys

def send_headers(status="200 OK", content_type="text/html; charset=utf-8"):
    sys.stdout.write(f"Status: {status}\r\n")
    sys.stdout.write(f"Content-Type: {content_type}\r\n\r\n")

method = os.environ.get("REQUEST_METHOD", "")
query  = os.environ.get("QUERY_STRING", "")

# --- Status code tests ---
if query == "status=404":
    send_headers("404 Not Found")
    sys.stdout.write("<h1>404 — Not Found (CGI Test)</h1>")
    sys.exit(0)

if query == "status=500":
    send_headers("500 Internal Server Error")
    sys.stdout.write("<h1>500 — Internal Server Error (CGI Test)</h1>")
    sys.exit(0)

if query == "redirect":
    sys.stdout.write("Status: 302 Found\r\n")
    sys.stdout.write("Location: https://google.com\r\n")
    sys.stdout.write("Content-Type: text/html; charset=utf-8\r\n\r\n")
    sys.exit(0)

# --- DELETE test ---
if method == "DELETE":
    send_headers("200 OK")
    sys.stdout.write("<h1>DELETE request received by CGI</h1>")
    sys.stdout.write(f"<p>Query string: {query}</p>")
    sys.exit(0)

# --- POST test ---
if method == "POST":
    length = int(os.environ.get("CONTENT_LENGTH", 0))
    body = sys.stdin.read(length)
    send_headers("200 OK")
    sys.stdout.write("<h1>POST request received by CGI</h1>")
    sys.stdout.write("<p>Body:</p><pre>{}</pre>".format(body))
    sys.stdout.write(f"<p>Query: {query}</p>")
    sys.exit(0)

# --- GET test (default) ---
send_headers("200 OK")

sys.stdout.write(f"""
<html>
<body>
<h1>CGI Test Suite</h1>

<h2>Detected method: {method}</h2>
<p>Query string: {query}</p>

<h3>Quick status tests:</h3>
<ul>
    <li><a href="?status=404">Test 404</a></li>
    <li><a href="?status=500">Test 500</a></li>
    <li><a href="?redirect">Test 302 Redirect</a></li>
</ul>

<h3>Test POST:</h3>
<form action="" method="POST">
    <textarea name="msg" rows="3" cols="40">Type something...</textarea><br>
    <button type="submit">Send POST</button>
</form>

<h3>Test DELETE (via curl):</h3>
<code>curl -X DELETE "http://localhost:8080/cgi-bin/test_cgi.py?delete=yes"</code>

</body>
</html>
""")
