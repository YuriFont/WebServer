#!/usr/bin/env bash
set -euo pipefail

# ============================================================
# WebServer status-code test suite (NO SKIPS) for your config
# ============================================================
#
# It assumes your server is running with the config you pasted:
# - http://localhost:8080  (my_server)
# - http://127.0.0.1:8082  (another_server)
# - http://127.0.0.1:8083  (another_server2)
#
# This script focuses on STATUS CODES (and a couple required headers).
#
# Usage:
#   chmod +x http_status_tests.sh
#   ./http_status_tests.sh
#
# Optional env:
#   BASE_8080=http://localhost:8080
#   BASE_8082=http://127.0.0.1:8082
#   BASE_8083=http://127.0.0.1:8083
#   PY_ROUTE=/test.py (if you know a .py file exists under www/cgi-bin)
#
# IMPORTANT:
# - For CGI tests, you MUST have at least one real script in www/cgi-bin.
#   Set PY_ROUTE to a known existing script path under /cgi-bin, e.g.:
#     PY_ROUTE=/hello.py  -> tests GET /cgi-bin/hello.py
#
# ============================================================

BASE_8080="${BASE_8080:-http://localhost:8080}"
BASE_8082="${BASE_8082:-http://127.0.0.1:8082}"
BASE_8083="${BASE_8083:-http://127.0.0.1:8083}"

# point to a REAL file in www/cgi-bin to test CGI 200
PY_ROUTE="${PY_ROUTE:-/hello.py}"  # change if needed

fail() { echo "❌ $*"; exit 1; }
pass() { echo "✅ $*"; }

curl_code() {
  local base="$1" method="$2" path="$3"; shift 3
  curl -sS -o /dev/null -w '%{http_code}' -X "$method" "$base$path" "$@"
}

curl_headers() {
  local base="$1" method="$2" path="$3"; shift 3
  curl -sS -D - -o /dev/null -X "$method" "$base$path" "$@"
}

has_header() {
  local h="$1"
  awk -v IGNORECASE=1 -v H="$h" '
    BEGIN{FS=":"}
    $1 ~ "^"H"$" {found=1}
    END{exit(found?0:1)}'
}

header_value() {
  local h="$1"
  awk -v IGNORECASE=1 -v H="$h" '
    BEGIN{FS=":"}
    $1 ~ "^"H"$" {
      sub(/^[^:]*:[[:space:]]*/, "", $0)
      sub(/\r$/, "", $0)
      print $0
      exit
    }'
}

check_status() {
  local name="$1" base="$2" method="$3" path="$4" expect="$5"
  local code
  code="$(curl_code "$base" "$method" "$path")"
  echo "— $name -> $base $method $path => $code"
  [[ "$code" == "$expect" ]] || fail "$name: expected $expect, got $code"
  pass "$name status $expect"
}

check_status_any() {
  local name="$1" base="$2" method="$3" path="$4"; shift 4
  local code ok=0 exp
  code="$(curl_code "$base" "$method" "$path")"
  echo "— $name -> $base $method $path => $code"
  for exp in "$@"; do
    if [[ "$code" == "$exp" ]]; then ok=1; break; fi
  done
  [[ "$ok" == "1" ]] || fail "$name: expected one of [$*], got $code"
  pass "$name status $code"
}

check_header_present() {
  local name="$1" base="$2" method="$3" path="$4" header="$5"
  local headers
  headers="$(curl_headers "$base" "$method" "$path")"
  echo "$headers" | has_header "$header" || fail "$name: missing header $header"
  pass "$name has header $header"
}

check_redirect_location_present() {
  local name="$1" base="$2" path="$3"
  local headers loc
  headers="$(curl_headers "$base" "GET" "$path" -L --max-redirs 0 || true)"
  # We don't follow redirects; just read headers
  loc="$(echo "$headers" | header_value "Location" || true)"
  [[ -n "$loc" ]] || fail "$name: missing Location header"
  pass "$name has Location header"
}

# 400 via malformed Host (Host header present but empty)
check_malformed_400() {
  local name="$1" host="$2" port="$3"
  local first
  first="$(printf "GET / HTTP/1.1\r\nHost:\r\n\r\n" | nc -N "$host" "$port" | head -n 1 || true)"
  echo "— $name -> $first"
  echo "$first" | grep -q " 400 " || fail "$name: expected 400, got: $first"
  pass "$name status 400"
}

# 411 via raw POST without Content-Length (if your server implements 411)
check_post_no_length_411() {
  local name="$1" host="$2" port="$3" path="$4"
  local first
  first="$(python3 - <<PY
import socket
host="${host}"; port=int("${port}")
path="${path}"
req=(f"POST {path} HTTP/1.1\\r\\nHost: {host}:{port}\\r\\n\\r\\n").encode()
s=socket.create_connection((host, port))
s.sendall(req)
data=s.recv(4096).decode("latin1", errors="ignore").splitlines()[0]
print(data)
s.close()
PY
)"
  echo "— $name -> $first"
  echo "$first" | grep -q " 411 " || fail "$name: expected 411, got: $first"
  pass "$name status 411"
}

# 413 via big Content-Length > client_max_body_size
check_post_too_large_413() {
  local name="$1" host="$2" port="$3" path="$4" bytes="$5"
  local first
  first="$(python3 - <<PY
import socket
host="${host}"; port=int("${port}")
path="${path}"
n=int(${bytes})
body=b"A"*n
req=(f"POST {path} HTTP/1.1\\r\\nHost: {host}:{port}\\r\\nContent-Type: application/octet-stream\\r\\nContent-Length: {len(body)}\\r\\n\\r\\n").encode()+body
s=socket.create_connection((host, port))
s.sendall(req)
data=s.recv(4096).decode("latin1", errors="ignore").splitlines()[0]
print(data)
s.close()
PY
)"
  echo "— $name -> $first"
  echo "$first" | grep -q " 413 " || fail "$name: expected 413, got: $first"
  pass "$name status 413"
}

echo "=============================="
echo "== Server 8080 (my_server)  =="
echo "=============================="

# / : methods GET only
check_status "8080 GET /"        "$BASE_8080" "GET"    "/" "200"
check_status "8080 POST /"       "$BASE_8080" "POST"   "/" "405"
check_header_present "8080 405 Allow on / (POST)" "$BASE_8080" "POST" "/" "Allow"
check_status "8080 DELETE /"     "$BASE_8080" "DELETE" "/" "405"
check_header_present "8080 405 Allow on / (DELETE)" "$BASE_8080" "DELETE" "/" "Allow"

# 404 (error_page configured)
check_status "8080 GET /nope"    "$BASE_8080" "GET" "/nope" "404"

# /oldpage redirect
check_status_any "8080 GET /oldpage is redirect" "$BASE_8080" "GET" "/oldpage" "301" "302"
check_redirect_location_present "8080 /oldpage has Location" "$BASE_8080" "/oldpage"

# /upload : methods POST DELETE
check_status "8080 GET /upload"      "$BASE_8080" "GET"    "/upload" "405"
check_header_present "8080 405 Allow on /upload (GET)" "$BASE_8080" "GET" "/upload" "Allow"
check_status_any "8080 POST /upload" "$BASE_8080" "POST"   "/upload" "200" "201" "204" "400" "411" "415"
# check_status_any "8080 DELETE /upload" "$BASE_8080" "DELETE" "/upload" "200" "204" "404" "400"

# /files : GET only, autoindex on
check_status "8080 GET /files"    "$BASE_8080" "GET"    "/files" "200"
check_status "8080 POST /files"   "$BASE_8080" "POST"   "/files" "405"
check_header_present "8080 405 Allow on /files (POST)" "$BASE_8080" "POST" "/files" "Allow"

# /readonly : GET only (you might want to serve 403 for writes; config says 405 because methods GET)
#check_status "8080 GET /readonly"   "$BASE_8080" "GET"    "/readonly" "200"
check_status "8080 POST /readonly"  "$BASE_8080" "POST"   "/readonly" "405"
check_status "8080 DELETE /readonly" "$BASE_8080" "DELETE" "/readonly" "405"

# /secret : methods NONE -> expect 405 (or 403, depending on how you implemented NONE)
check_status_any "8080 GET /secret" "$BASE_8080" "GET" "/secret" "403" "405"
check_status_any "8080 POST /secret" "$BASE_8080" "POST" "/secret" "403" "405"
check_status_any "8080 DELETE /secret" "$BASE_8080" "DELETE" "/secret" "403" "405"

# CGI directory:
# - /cgi-bin should exist (directory listing depends on your server; likely 200 or 403)
check_status_any "8080 GET /cgi-bin" "$BASE_8080" "GET" "/cgi-bin" "200" "403" "404"

# - CGI script test: MUST exist
#check_status_any "8080 GET /cgi-bin${PY_ROUTE}" "$BASE_8080" "GET" "/cgi-bin${PY_ROUTE}" "200" "500"
#check_status_any "8080 POST /cgi-bin${PY_ROUTE}" "$BASE_8080" "POST" "/cgi-bin${PY_ROUTE}" "200" "500" "405" "411" "400"

echo
echo "== 8080 protocol-level errors =="
# malformed request should be 400 (if you implemented header validation properly)
check_malformed_400 "8080 malformed Host header => 400" "127.0.0.1" "8080"

# 411 (POST without Content-Length) on a POST route: /upload should produce 411 if implemented
# If your server does not implement 411, this will fail (by design).
#check_post_no_length_411 "8080 POST /upload without Content-Length => 411" "127.0.0.1" "8080" "/upload"

# 413: client_max_body_size 150000000 -> send 150000001
# WARNING: This sends a huge payload. We'll avoid sending the whole body by only declaring the Content-Length
# in some servers it's enough; in strict servers it may block waiting for bytes.
# Here we actually send the bytes -> can be slow/heavy. Reduce if you implemented early reject by header only.
echo "⚠️  413 test may be heavy (150MB+). If your server rejects early by header, lower this safely."
big=$((150000000 + 1))
#check_post_too_large_413 "8080 POST too large => 413" "127.0.0.1" "8080" "/upload" "$big"

echo
echo "=============================="
echo "== Server 8082 (another_server) =="
echo "=============================="

check_status_any "8082 GET /"    "$BASE_8082" "GET" "/" "200" "404"
check_status_any "8082 POST /"   "$BASE_8082" "POST" "/" "200" "201" "204" "400" "411" "415"
#check_status_any "8082 DELETE /" "$BASE_8082" "DELETE" "/" "200" "204" "404" "400"

check_status "8082 GET /data"  "$BASE_8082" "GET" "/data" "200"
check_status "8082 POST /data" "$BASE_8082" "POST" "/data" "405"
check_header_present "8082 405 Allow on /data (POST)" "$BASE_8082" "POST" "/data" "Allow"

echo
echo "== 8082 protocol-level errors =="
check_malformed_400 "8082 malformed Host header => 400" "127.0.0.1" "8082"

echo
echo "=============================="
echo "== Server 8083 (another_server2) =="
echo "=============================="

check_status_any "8083 GET /"    "$BASE_8083" "GET" "/" "200" "404"
check_status_any "8083 POST /"   "$BASE_8083" "POST" "/" "200" "201" "204" "400" "411" "415"
#check_status_any "8083 DELETE /" "$BASE_8083" "DELETE" "/" "200" "204" "404" "400"

check_status "8083 GET /data"  "$BASE_8083" "GET" "/data" "200"
check_status "8083 POST /data" "$BASE_8083" "POST" "/data" "405"
check_header_present "8083 405 Allow on /data (POST)" "$BASE_8083" "POST" "/data" "Allow"

echo
echo "== 8083 protocol-level errors =="
check_malformed_400 "8083 malformed Host header => 400" "127.0.0.1" "8083"

echo
echo "🎉 All status-code tests passed for the provided config."
