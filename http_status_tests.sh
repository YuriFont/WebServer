#!/usr/bin/env bash
set -euo pipefail

# =========================
# Minimal HTTP status suite
# Focus: status codes + a few critical headers (Allow/Location)
# =========================

BASE_URL="${BASE_URL:-http://localhost:8080}"

# ---- Adjust these to match your config ----
EXISTING_PATH="${EXISTING_PATH:-/}"            # must return 200
NOT_FOUND_PATH="${NOT_FOUND_PATH:-/nope}"      # must return 404
METHOD_405_PATH="${METHOD_405_PATH:-/}"        # DELETE here must return 405
FORBIDDEN_PATH="${FORBIDDEN_PATH:-}"           # set to "/something" to test 403, or leave empty to skip
REDIRECT_PATH="${REDIRECT_PATH:-}"             # set to "/redirect" to test 301/302, or leave empty to skip
POST_PATH="${POST_PATH:-}"                     # set to "/upload" to test 411/413, or leave empty to skip
CGI_OK_PATH="${CGI_OK_PATH:-}"                 # set to "/cgi-bin/test.py" to test 200, or leave empty to skip
CGI_FAIL_PATH="${CGI_FAIL_PATH:-}"             # set to "/cgi-bin/fail.py" to test 500, or leave empty to skip
BODY_LIMIT_BYTES="${BODY_LIMIT_BYTES:-0}"      # set >0 to test 413

# Parse host/port from BASE_URL (expects http://host:port)
HOST="$(echo "$BASE_URL" | sed -E 's#^https?://([^:/]+).*#\1#')"
PORT="$(echo "$BASE_URL" | sed -E 's#^https?://[^:/]+:([0-9]+).*#\1#')"
[[ -n "$HOST" && -n "$PORT" ]] || { echo "BASE_URL must include host:port (e.g. http://localhost:8080)"; exit 1; }

# ---------- helpers ----------
fail() { echo "❌ $*"; exit 1; }
pass() { echo "✅ $*"; }

skip_if_empty() {
  local v="$1" msg="$2"
  if [[ -z "$v" ]]; then
    echo "⏭️  Skipping: $msg"
    return 0
  fi
  return 1
}

curl_code() {
  # args: method path extra...
  local method="$1"; shift
  local path="$1"; shift
  curl -sS -o /dev/null -w '%{http_code}' -X "$method" "$BASE_URL$path" "$@"
}

curl_headers() {
  local method="$1"; shift
  local path="$1"; shift
  curl -sS -D - -o /dev/null -X "$method" "$BASE_URL$path" "$@"
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

has_header() {
  local h="$1"
  awk -v IGNORECASE=1 -v H="$h" '
    BEGIN{FS=":"}
    $1 ~ "^"H"$" {found=1}
    END{exit(found?0:1)}'
}

check_status() {
  local name="$1" method="$2" path="$3" expect="$4"
  local code
  code="$(curl_code "$method" "$path")"
  echo "— $name -> $method $path => $code"
  [[ "$code" == "$expect" ]] || fail "$name: expected $expect, got $code"
  pass "$name status $expect"
}

check_status_any() {
  local name="$1" method="$2" path="$3"; shift 3
  local code ok=0 exp
  code="$(curl_code "$method" "$path")"
  echo "— $name -> $method $path => $code"
  for exp in "$@"; do
    if [[ "$code" == "$exp" ]]; then ok=1; break; fi
  done
  [[ "$ok" == "1" ]] || fail "$name: expected one of [$*], got $code"
  pass "$name status $code"
}

check_header_present() {
  local name="$1" method="$2" path="$3" header="$4"
  local headers
  headers="$(curl_headers "$method" "$path")"
  echo "$headers" | has_header "$header" || fail "$name: missing header $header"
  pass "$name has header $header"
}

# ---------- raw request tests (for malformed / no Content-Length) ----------
check_malformed_400() {
  local name="$1"
  local first
  first="$(printf "GET / HTTP/1.1\r\nHost:\r\n\r\n" | nc -N "$HOST" "$PORT" | head -n 1 || true)"
  echo "— $name -> $first"
  echo "$first" | grep -q " 400 " || fail "$name: expected 400, got: $first"
  pass "$name status 400"
}

check_post_no_length() {
  local name="$1" path="$2" expect="$3"
  local first
  first="$(python3 - <<PY
import socket
host="${HOST}"; port=int("${PORT}")
path="${path}"
req=(f"POST {path} HTTP/1.1\\r\\nHost: ${HOST}\\r\\n\\r\\n").encode()
s=socket.create_connection((host, port))
s.sendall(req)
data=s.recv(4096).decode("latin1", errors="ignore").splitlines()[0]
print(data)
s.close()
PY
)"
  echo "— $name -> $first"
  echo "$first" | grep -q " $expect " || fail "$name: expected $expect, got: $first"
  pass "$name status $expect"
}

check_post_too_large_413() {
  local name="$1" path="$2" bytes="$3"
  local first
  first="$(python3 - <<PY
import socket
host="${HOST}"; port=int("${PORT}")
path="${path}"
n=int(${bytes})
body=b"A"*n
req=(f"POST {path} HTTP/1.1\\r\\nHost: ${HOST}\\r\\nContent-Type: application/octet-stream\\r\\nContent-Length: {len(body)}\\r\\n\\r\\n").encode()+body
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

# =========================
# Tests
# =========================

echo "== Basic =="
check_status "GET existing path" "GET" "$EXISTING_PATH" "200"
check_status "GET missing path"  "GET" "$NOT_FOUND_PATH" "404"

echo
echo "== 405 + Allow =="
check_status "DELETE returns 405" "DELETE" "$METHOD_405_PATH" "405"
check_header_present "405 has Allow header" "DELETE" "$METHOD_405_PATH" "Allow"

# echo
# echo "== 400 (malformed request) =="
# check_malformed_400 "Malformed request returns 400"

echo
echo "== Optional: 403 Forbidden =="
if ! skip_if_empty "$FORBIDDEN_PATH" "403 test (FORBIDDEN_PATH empty)"; then
  check_status "Forbidden path" "GET" "$FORBIDDEN_PATH" "403"
fi

echo
echo "== Optional: Redirect 301/302 =="
if ! skip_if_empty "$REDIRECT_PATH" "redirect test (REDIRECT_PATH empty)"; then
  check_status_any "Redirect path" "GET" "$REDIRECT_PATH" "301" "302"
  check_header_present "Redirect has Location header" "GET" "$REDIRECT_PATH" "Location"
fi

echo
echo "== Optional: 411 Length Required =="
if ! skip_if_empty "$POST_PATH" "411 test (POST_PATH empty)"; then
  # If you don't implement 411, change expected code here.
  check_post_no_length "POST without Content-Length" "$POST_PATH" "411"
fi

echo
echo "== Optional: 413 Payload Too Large =="
if [[ -n "$POST_PATH" && "${BODY_LIMIT_BYTES}" != "0" ]]; then
  big_bytes=$((BODY_LIMIT_BYTES + 1))
  check_post_too_large_413 "POST too large" "$POST_PATH" "$big_bytes"
else
  echo "⏭️  Skipping: 413 test (POST_PATH empty or BODY_LIMIT_BYTES=0)"
fi

echo
echo "== Optional: CGI =="
if ! skip_if_empty "$CGI_OK_PATH" "CGI ok test (CGI_OK_PATH empty)"; then
  check_status "CGI ok" "GET" "$CGI_OK_PATH" "200"
fi
if ! skip_if_empty "$CGI_FAIL_PATH" "CGI fail test (CGI_FAIL_PATH empty)"; then
  check_status "CGI fail" "GET" "$CGI_FAIL_PATH" "500"
fi

echo
echo "🎉 Status code checks passed."
