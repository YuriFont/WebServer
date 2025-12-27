#!/usr/bin/env python3
# delete.py -- CGI que remove item do db.json por id (query string ?id=)

import os, sys, json, urllib.parse

HERE = os.path.dirname(__file__)
LIB = os.path.normpath(os.path.join(HERE, "..", "lib"))
if LIB not in sys.path:
    sys.path.insert(0, LIB)
    
from db import read_db, write_db

def respond(status_code, payload, content_type="application/json"):
    print(f"Status: {status_code}")
    print(f"Content-Type: {content_type}")
    print()
    if isinstance(payload, (dict, list)):
        print(json.dumps(payload, indent=2, ensure_ascii=False))
    else:
        print(payload)

def parse_id_from_body():
    try:
        length = int(os.environ.get("CONTENT_LENGTH", "0"))
    except:
        length = 0
    if length <= 0:
        return None
    body = sys.stdin.read(length)
    try:
        j = json.loads(body)
        return int(j.get("id"))
    except:
        return None

def main():
    method = os.environ.get("REQUEST_METHOD", "GET").upper()
    # muitos setups disparam DELETE; alguns usam POST com _method=DELETE
    if method not in ("DELETE", "POST"):
        respond("405 Method Not Allowed", {"error": "Use DELETE or POST with id in body/query"})
        return

    qs = os.environ.get("QUERY_STRING", "")
    params = urllib.parse.parse_qs(qs)
    id_val = None
    if "id" in params:
        try:
            id_val = int(params["id"][0])
        except:
            respond("400 Bad Request", {"error": "id must be integer"})
            return
    else:
        # tenta ler do body JSON
        id_val = parse_id_from_body()

    if id_val is None:
        respond("400 Bad Request", {"error": "id required (query or json body)"})
        return

    items = read_db()
    new_items = []
    removed = None
    for it in items:
        try:
            if int(it.get("id", -1)) == id_val:
                removed = it
                continue
        except:
            pass
        new_items.append(it)

    if removed is None:
        respond("404 Not Found", {"error": "Item not found"})
        return

    write_db(new_items)
    respond("200 OK", {"deleted": removed})

if __name__ == "__main__":
    main()
