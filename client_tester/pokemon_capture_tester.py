import requests
import json

# **Configura l'URL del server CGI**
CGI_URL = "http://localhost:8080/pokemon_capture.py"  # Modifica se necessario

# **Dati della richiesta**
payload = {
    "pokemon": "Skarmory",   # Cambia il Pokémon per testare altre specie
    "pokeball": "Poke Ball"  # Prova con "Poké Ball", "Great Ball", "Master Ball", ecc.
}

# **Invia la richiesta POST**
try:
    response = requests.post(CGI_URL, json=payload)

    # **Verifica lo stato della risposta**
    if response.status_code == 200:
        print("\n✅ **Risposta ricevuta!**")
        print(json.dumps(response.json(), indent=4))
    else:
        print(f"\n❌ Errore HTTP {response.status_code}")
        print(response.text)

except requests.exceptions.RequestException as e:
    print(f"\n❌ Errore di connessione: {e}")
