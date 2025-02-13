import requests
import json

# **Configurazione**
CGI_URL = "http://localhost:8080/pokemon_battle.py"  # Cambia se necessario

# **Dati da inviare**
data = {
    "pokemon1": {"name": "Pikachu", "hp": 100},
    "pokemon2": {"name": "Charmander", "hp": 100},
    "mossa1": {"name": "Fulmine", "power": 55},
    "mossa2": {"name": "Lanciafiamme", "power": 50}
}

# **Invio della richiesta POST**
try:
    response = requests.post(CGI_URL, json=data)
    
    # **Stampa del risultato**
    if response.status_code == 200:
        risultato = response.json()
        print("\n🛡️ **Risultato della Battaglia Pokémon** 🛡️\n")
        print(f"🏆 **Vincitore:** {risultato['vincitore']}\n")
        print("📜 **Log della battaglia:**")
        for entry in risultato["log"]:
            print(f"➡️ {entry}")
    else:
        print("❌ Errore nella richiesta. Codice:", response.status_code)
        print(response.text)
except Exception as e:
    print("❌ Errore nella connessione:", e)
