#!/usr/bin/env python3
# db.py -- helper para ler/gravar db.json com locking e escrita atômica

import os
import json
import fcntl
import tempfile

DB_PATH = os.path.join(os.path.dirname(__file__), "db.json")

def ensure_db_exists():
    if not os.path.exists(DB_PATH):
        with open(DB_PATH, "w") as f:
            json.dump([], f)

def read_db():
    ensure_db_exists()
    with open(DB_PATH, "r") as f:
        # shared lock para leitura
        fcntl.flock(f.fileno(), fcntl.LOCK_SH)
        try:
            try:
                data = json.load(f)
            except ValueError:
                data = []
        finally:
            fcntl.flock(f.fileno(), fcntl.LOCK_UN)
    return data

def write_db(data):
    """
    Escreve de forma atômica usando tempfile + os.replace e lock exclusivo.
    """
    ensure_db_exists()
    # cria temp no mesmo diretório para que os.replace seja atômico
    dname = os.path.dirname(DB_PATH)
    fd, tmp_path = tempfile.mkstemp(dir=dname, prefix=".dbtmp")
    try:
        with os.fdopen(fd, "w") as tmp:
            json.dump(data, tmp, indent=2, ensure_ascii=False)
            tmp.flush()
            os.fsync(tmp.fileno())
        # lock exclusivo enquanto substitui (abrindo arquivo destino)
        with open(DB_PATH, "r+") as f:
            fcntl.flock(f.fileno(), fcntl.LOCK_EX)
            try:
                os.replace(tmp_path, DB_PATH)  # atômico em POSIX
            finally:
                fcntl.flock(f.fileno(), fcntl.LOCK_UN)
    finally:
        if os.path.exists(tmp_path):
            try:
                os.remove(tmp_path)
            except:
                pass

def generate_next_id(items):
    # espera que itens sejam lista de dicts com campo 'id' numérico
    max_id = 0
    for it in items:
        try:
            iid = int(it.get("id", 0))
            if iid > max_id: max_id = iid
        except:
            pass
    return max_id + 1
