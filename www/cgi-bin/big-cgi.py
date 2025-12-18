#!/usr/bin/env python3
import sys

# Cabeçalhos HTTP obrigatórios para CGI
sys.stdout.write("Status: 200 OK\r\n")
sys.stdout.write("Content-Type: text/plain\r\n")
sys.stdout.write("\r\n")

# Corpo da resposta (> 5000 bytes)
line = "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789\n"  # 37 bytes aprox
repeat = 150000  # 37 * 150 ≈ 5550 bytes

for i in range(repeat):
    sys.stdout.write(f"{i:03d} - {line}")
