#!/usr/bin/env python3

import sys
import time

# Scrive un header minimo, se vuoi simulare un output CGI.
print("Content-Type: text/plain\r\n\r\n", end="")

# Entra in loop infinito, stampando un messaggio ogni 2 secondi
# In modo da non saturare la CPU al 100%.
counter = 0
while True:
    counter += 1
    print(f"Loop iteration #{counter}...")
    sys.stdout.flush()  # forza lo scaricamento del buffer
    time.sleep(2)