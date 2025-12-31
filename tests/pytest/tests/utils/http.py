# tests./utils/http.py
import requests

def get(path, host="localhost", port=8080):
    return requests.get(f"http://{host}:{port}{path}")

def post(path, data=None, host="localhost", port=8080):
    return requests.post(f"http://{host}:{port}{path}", data=data)

def delete(path, host="localhost",port=8080):
    return requests.delete(f"http://{host}:{port}{path}")