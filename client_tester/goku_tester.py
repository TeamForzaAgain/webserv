import requests

# Configurazione
SERVER_URL = "http://localhost:8080/goku/evcecvevcre.png"  # Modifica con il tuo server
FILE_PATH = "goku.png"  # Il file da inviare

def upload_file():
    """Invia il file goku.png come multipart/form-data"""
    try:
        with open(FILE_PATH, "rb") as file:
            files = {"file": (FILE_PATH, file, "image/png")}
            response = requests.post(SERVER_URL, files=files)

        print("📤 File inviato!")
        print("📥 Risposta del server:", response.status_code)
        print(response.text)

    except FileNotFoundError:
        print(f"❌ Il file {FILE_PATH} non esiste nella directory corrente.")
    except Exception as e:
        print(f"❌ Errore durante l'upload: {e}")

if __name__ == "__main__":
    upload_file()
