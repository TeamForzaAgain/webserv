import socket
import time

SERVER_HOST = "localhost"
SERVER_PORT = 8080

def test_persistent_connection(timeout=30):
    """
    Testa una connessione persistente al server.
    1. Si connette al server
    2. Invia una richiesta HTTP
    3. Aspetta senza inviare nulla per `timeout` secondi
    4. Tenta di inviare un'altra richiesta e verifica se la connessione è ancora aperta
    """
    print(f"\n🔹 Test: Connessione persistente con timeout {timeout}s")

    # Creazione del socket
    client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    client_socket.settimeout(5)  # Timeout breve per evitare blocchi

    try:
        # Connessione al server
        client_socket.connect((SERVER_HOST, SERVER_PORT))
        print("✅ Connessione stabilita")

        # Invio della prima richiesta
        request = "GET / HTTP/1.1\r\nHost: localhost\r\nConnection: keep-alive\r\n\r\n"
        client_socket.sendall(request.encode())
        response = client_socket.recv(4096)
        print("✅ Risposta ricevuta (prima richiesta):", response.decode().split("\r\n")[0])

        # Aspetta senza inviare nulla (per simulare inattività)
        print(f"⏳ Aspetto {timeout} secondi per testare la chiusura per inattività...")
        time.sleep(timeout)

        # Tenta di inviare una seconda richiesta
        try:
            client_socket.sendall(request.encode())
            response = client_socket.recv(4096)
            print("✅ Connessione ancora aperta, risposta ricevuta:", response.decode().split("\r\n")[0])
        except (socket.error, socket.timeout):
            print("❌ Connessione chiusa dal server per inattività")
        
    except Exception as e:
        print("❌ Errore nella connessione:", e)
    
    finally:
        client_socket.close()
        print("🔚 Socket chiuso")

# **Esegui il test con un timeout più breve del limite del server**
test_persistent_connection(timeout=65)  # Dovrebbe essere chiuso dal server