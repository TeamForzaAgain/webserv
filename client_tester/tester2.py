import socket
import time

# Configura il tuo server (modifica se necessario)
SERVER_HOST = "127.0.0.1"
SERVER_PORT = 8080

# 🟢 **Test 1: Content-Length incompleto**
def test_content_length_incomplete():
    print("\n🔹 TEST 1: Content-Length incompleto\n")
    client = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    client.connect((SERVER_HOST, SERVER_PORT))

    request = (
        "POST /upload HTTP/1.1\r\n"
        "Host: localhost\r\n"
        "Content-Length: 10\r\n"
        "Connection: keep-alive\r\n"
        "\r\n"
        "Hel"
    )

    print("🟡 Inviando prima parte...")
    client.sendall(request.encode())
    time.sleep(2)  # Simula ritardo nella trasmissione

    print("🟡 Inviando seconda parte...")
    client.sendall("loWorld".encode())  # Completa il body

    response = client.recv(1024).decode()
    print("🔽 Risposta ricevuta:\n", response)
    client.close()

# 🟢 **Test 2: Chunked Transfer Encoding**
def test_chunked_transfer_encoding():
    print("\n🔹 TEST 2: Chunked Transfer Encoding\n")
    client = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    client.connect((SERVER_HOST, SERVER_PORT))

    request = (
        "POST /upload HTTP/1.1\r\n"
        "Host: localhost\r\n"
        "Transfer-Encoding: chunked\r\n"
        "Connection: keep-alive\r\n"
        "\r\n"
        "5\r\n"
        "Hello\r\n"
    )

    print("🟡 Inviando primo chunk...")
    client.sendall(request.encode())
    time.sleep(2)

    print("🟡 Inviando secondo chunk...")
    client.sendall("6\r\nWorld!\r\n".encode())
    time.sleep(2)

    print("🟡 Inviando terminazione chunked...")
    client.sendall("0\r\n\r\n".encode())

    response = client.recv(1024).decode()
    print("🔽 Risposta ricevuta:\n", response)
    client.close()

# 🟢 **Test 3: Request Completa**
def test_complete_request():
    print("\n🔹 TEST 3: Request Completa\n")
    client = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    client.connect((SERVER_HOST, SERVER_PORT))

    request = (
        "POST /upload HTTP/1.1\r\n"
        "Host: localhost\r\n"
        "Content-Length: 11\r\n"
        "Connection: close\r\n"
        "\r\n"
        "Hello World"
    )

    print("🟡 Inviando request completa...")
    client.sendall(request.encode())

    response = client.recv(1024).decode()
    print("🔽 Risposta ricevuta:\n", response)
    client.close()

# **Esegui i test**
if __name__ == "__main__":
    test_content_length_incomplete()
    test_chunked_transfer_encoding()
    test_complete_request()