#!/usr/bin/env python3
# post.py -- CGI que adiciona um item JSON ao db.json e retorna o item criado
# Validações: body vazio => 400, JSON inválido => 400, falta de name ou value => 400

import os
import sys
import json

HERE = os.path.dirname(__file__)
LIB = os.path.normpath(os.path.join(HERE, "..", "lib"))
if LIB not in sys.path:
    sys.path.insert(0, LIB)
    
from db import read_db, write_db, generate_next_id

def respond(status_code, payload, content_type="application/json"):
    # status header opcional para CGI
    print(f"Status: {status_code}")
    print(f"Content-Type: {content_type}; charset=utf-8")
    print()  # fim dos headers
    if isinstance(payload, (dict, list)):
        print(json.dumps(payload, indent=2, ensure_ascii=False))
    else:
        print(payload)

def validate_payload(obj):
    """
    Verifica se obj tem 'name' e 'value' com conteúdo válido.
    Retorna (True, None) se ok, ou (False, dict) com erro detalhado.
    """
    missing = []
    empty = []

    # verifica presença de chave
    if 'name' not in obj:
        missing.append('name')
    else:
        # considera inválido se vazio ou somente espaços
        try:
            if isinstance(obj['name'], str):
                if obj['name'].strip() == "":
                    empty.append('name')
            elif obj['name'] is None:
                empty.append('name')
        except Exception:
            empty.append('name')

    if 'value' not in obj:
        missing.append('value')
    else:
        # aceita number ou string não vazia; considere ajustar se quiser forçar number
        if obj['value'] is None:
            empty.append('value')
        else:
            if isinstance(obj['value'], str) and obj['value'].strip() == "":
                empty.append('value')

    if missing or empty:
        detail = {}
        if missing:
            detail['missing'] = missing
        if empty:
            detail['empty'] = empty
        return False, {
            "error": "Invalid payload",
            "message": "Campos ausentes ou vazios",
            "details": detail
        }
    return True, None

def main():
    method = os.environ.get("REQUEST_METHOD", "GET").upper()
    if method != "POST":
        respond("405 Method Not Allowed", {"error": "Use POST"})
        return

    try:
        length = int(os.environ.get("CONTENT_LENGTH", "0"))
    except Exception:
        length = 0

    body = sys.stdin.read(length) if length > 0 else ""
    if not body:
        respond("400 Bad Request", {"error": "Empty body", "message": "O corpo da requisição está vazio"})
        return

    try:
        obj = json.loads(body)
        if not isinstance(obj, dict):
            raise ValueError("JSON root must be an object")
    except Exception:
        respond("400 Bad Request", {"error": "Invalid JSON body", "message": "Corpo não é um JSON válido"})
        return

    ok, err = validate_payload(obj)
    if not ok:
        respond("400 Bad Request", err)
        return

    # Todos os checks ok -> inserir no DB
    items = read_db()
    new_id = generate_next_id(items)
    obj["id"] = new_id
    items.append(obj)
    write_db(items)

    respond("201 Created", obj)

if __name__ == "__main__":
    main()
