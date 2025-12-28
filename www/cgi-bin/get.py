#!/usr/bin/env python3
# get.py -- CGI que retorna todos os items ou um item por id (query string ?id=)

import os, sys, json, urllib.parse

HERE = os.path.dirname(__file__)
LIB = os.path.normpath(os.path.join(HERE, "..", "lib"))
if LIB not in sys.path:
    sys.path.insert(0, LIB)
    
from db import read_db

def respond(status_code, payload, content_type="application/json"):
    print(f"Status: {status_code}")
    print(f"Content-Type: {content_type}")
    print()
    if isinstance(payload, (dict, list)):
        print(json.dumps(payload, indent=2, ensure_ascii=False))
    else:
        print(payload)

def main():
    method = os.environ.get("REQUEST_METHOD", "GET").upper()
    if method != "GET":
        respond("405 Method Not Allowed", {"error": "Use GET"})
        return

    qs = os.environ.get("QUERY_STRING", "")
    params = urllib.parse.parse_qs(qs)
    items = read_db()

    if "id" in params:
        try:
            want = int(params["id"][0])
        except:
            respond("400 Bad Request", {"error": "id must be integer"})
            return
        for it in items:
            try:
                if int(it.get("id", -1)) == want:
                    respond("200 OK", it)
                    return
            except:
                continue
        respond("404 Not Found", {"error": "Item not found"})
    else:
        respond("200 OK", items)

if __name__ == "__main__":
    main()
