#!/usr/bin/env python3

import os
import sys
import json
import random
import requests

# **Mappatura delle Poké Ball con i loro moltiplicatori di cattura**
POKEBALL_RATES = {
    "Poke Ball": 1.0,
    "Great Ball": 1.5,
    "Ultra Ball": 2.0,
    "Master Ball": 255.0  # La Master Ball ha un tasso di cattura garantito
}

def get_pokemon_capture_rate(pokemon_name):
    """Ottiene il tasso di cattura di un Pokémon dalla PokéAPI."""
    api_url = f"https://pokeapi.co/api/v2/pokemon-species/{pokemon_name.lower()}"
    response = requests.get(api_url)
    if response.status_code == 200:
        data = response.json()
        return data.get("capture_rate", 0)
    else:
        return None

def calculate_catch_probability(capture_rate, ball_modifier):
    """
    Calcola la probabilità di cattura basata sul tasso di cattura e il moltiplicatore della Poké Ball.
    La formula è semplificata per scopi illustrativi.
    """
    # Formula semplificata per la probabilità di cattura
    catch_value = ((3 * 100 - 2 * 100) * capture_rate * ball_modifier) / (3 * 100)
    catch_probability = min((catch_value / 255), 1.0)  # La probabilità non può superare 1 (100%)
    return catch_probability

def simulate_catch(pokemon, pokeball):
    """Simula la cattura di un Pokémon e restituisce un dizionario con il risultato."""
    capture_rate = get_pokemon_capture_rate(pokemon)
    if capture_rate is None:
        return {
            "error": True,
            "message": f"Impossibile ottenere il tasso di cattura per {pokemon}."
        }

    ball_modifier = POKEBALL_RATES.get(pokeball, 1.0)  # Default: Poké Ball
    catch_probability = calculate_catch_probability(capture_rate, ball_modifier)

    # Simula la cattura
    caught = random.random() < catch_probability

    return {
        "pokemon": pokemon,
        "pokeball": pokeball,
        "caught": caught,
        "message": f"Hai catturato {pokemon}!" if caught else f"{pokemon} si e' liberato!",
        "catch_probability": round(catch_probability * 100, 2)  # Percentuale
    }

# **Legge il body della richiesta HTTP dalla STDIN**
try:
    content_length = int(os.environ.get("CONTENT_LENGTH", 0))
    request_body = sys.stdin.read(content_length)
    request_data = json.loads(request_body)

    # **Estrae i dati dalla richiesta**
    pokemon = request_data.get("pokemon", "Unknown")
    pokeball = request_data.get("pokeball", "Poke Ball")
    #stampa sullo standard error la pokemon e la pokeball


    # **Simula la cattura**
    result = simulate_catch(pokemon, pokeball)

    # **Stampa l'output in formato CGI**
    print("Content-Type: application/json\r")
    print("\r")  # Separatore tra header e body
    print(json.dumps(result, indent=4))

except Exception as e:
    print("Content-Type: application/json\r")
    print("\r")
    print(json.dumps({"error": "Bad Request", "message": str(e)}))
