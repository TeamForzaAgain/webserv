import socket

def send_http_request(host, port, server_name):
    """Invia una richiesta GET al web server e stampa la risposta"""
    request = f"GET / HTTP/1.1\r\nHost: {server_name}\r\nConnection: close\r\n\r\n"

    try:
        # Creiamo un socket TCP
        s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        s.connect((host, port))

        # Inviamo la richiesta HTTP
        s.sendall(request.encode())

        # Riceviamo la risposta
        response = s.recv(4096)  # Leggiamo fino a 4KB di risposta
        s.close()

        print(f"--- Response from {server_name} ({host}:{port}) ---")
        print(response.decode())
        print("--------------------------------------------------\n")

    except Exception as e:
        print(f"Error connecting to {server_name} ({host}:{port}): {e}")

# **TEST dei due server**
send_http_request("0.0.0.0", 8080, "Server8080")   # Test su Server8080 (any IP)
send_http_request("127.0.0.1", 8080, "Server8080_2")  # Test su Server8080_2 (localhost)
