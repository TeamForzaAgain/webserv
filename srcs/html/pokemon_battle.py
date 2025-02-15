#!/usr/bin/env python

import os
import sys
import json
import random

sys.stderr.write("DEBUG: Lo script è stato avviato!\n")
sys.stderr.flush()

# Funzione per calcolare il danno di un attacco
def calcola_danno(attaccante, mossa):
    base_power = mossa.get("power", 40)
    critico = 1.5 if random.random() < 0.1 else 1.0  # 10% di possibilità di colpo critico
    danno = base_power * critico
    return int(danno)

# Funzione per eseguire la battaglia Pokémon
def battaglia_pokemon(data):
    pokemon1 = data.get("pokemon1", {"name": "Pikachu", "hp": 100})
    pokemon2 = data.get("pokemon2", {"name": "Charmander", "hp": 100})
    
    mossa1 = data.get("mossa1", {"name": "Thunderbolt", "power": 50})
    mossa2 = data.get("mossa2", {"name": "Flamethrower", "power": 50})

    # Simuliamo i turni di combattimento
    log = []
    
    while pokemon1["hp"] > 0 and pokemon2["hp"] > 0:
        # Attacco Pokémon 1
        danno1 = calcola_danno(pokemon1, mossa1)
        pokemon2["hp"] -= danno1
        log.append(f"{pokemon1['name']} usa {mossa1['name']}! Danno: {danno1}")
        
        if pokemon2["hp"] <= 0:
            log.append(f"{pokemon2['name']} è stato sconfitto!")
            break
        
        # Attacco Pokémon 2
        danno2 = calcola_danno(pokemon2, mossa2)
        pokemon1["hp"] -= danno2
        log.append(f"{pokemon2['name']} usa {mossa2['name']}! Danno: {danno2}")
        
        if pokemon1["hp"] <= 0:
            log.append(f"{pokemon1['name']} è stato sconfitto!")
            break

    vincitore = pokemon1["name"] if pokemon1["hp"] > 0 else pokemon2["name"]

    return {
        "vincitore": vincitore,
        "log": log
    }

# **Lettura del body da STDIN (per richieste POST)**
try:
    content_length = int(os.environ.get("CONTENT_LENGTH", 0))
    request_body = sys.stdin.read(content_length)
    request_data = json.loads(request_body)
except Exception as e:
    request_data = {}

    # Header HTTP formattati correttamente
    sys.stdout.write("Status: 400 Bad Request\r\n")
    sys.stdout.write("Content-Type: application/json\r\n")
    sys.stdout.write("\r\n")  # Separazione header-body
    sys.stdout.write(json.dumps({"error": "Errore nella lettura della richiesta", "message": str(e)}, indent=4) + "\n")
    sys.exit(1)

# **Esegui la battaglia Pokémon**
risultato = battaglia_pokemon(request_data)

# **Stampa l'output CGI con header formattati**
sys.stdout.write("Status: 200 OK\r\n")
sys.stdout.write("Content-Type: application/json\r\n")
sys.stdout.write("\r\n")  # Separazione header-body
sys.stdout.write(json.dumps(risultato, indent=4) + "\n")
