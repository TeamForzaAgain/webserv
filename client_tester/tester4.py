import socket
import time

def send_content_length_request():
    host = "localhost"
    port = 8080
    
    content = "A" * 5000  # 5000 caratteri per testare frammentazione
    request = ("POST /upload HTTP/1.1\r\n"
               "Host: localhost\r\n"
               "Content-Length: 5000\r\n"
               "Connection: keep-alive\r\n\r\n")
    
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    sock.connect((host, port))
    sock.sendall(request.encode())
    time.sleep(0.5)
    
    for i in range(0, len(content), 512):
        sock.sendall(content[i:i+512].encode())
        time.sleep(0.1)  # Simula pacchetti frammentati
    
    response = sock.recv(4096)
    print("\nResponse for Content-Length:\n", response.decode())
    sock.close()

def send_chunked_request():
    host = "localhost"
    port = 8080
    
    request = ("POST /upload HTTP/1.1\r\n"
               "Host: localhost\r\n"
               "Transfer-Encoding: chunked\r\n"
               "Connection: keep-alive\r\n\r\n")
    
    chunks = [
        "100\r\n" + "A" * 256 + "\r\n",  # 256 byte chunk
        "80\r\n" + "B" * 128 + "\r\n",  # 128 byte chunk
        "200\r\n" + "C" * 512 + "\r\n",  # 512 byte chunk
        "50\r\n" + "D" * 80 + "\r\n",  # 80 byte chunk
        "1E\r\n" + "E" * 30 + "\r\n",  # 30 byte chunk
        "0\r\n\r\n"  # Terminazione
    ]
    
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    sock.connect((host, port))
    sock.sendall(request.encode())
    time.sleep(0.5)
    
    for chunk in chunks:
        sock.sendall(chunk.encode())
        time.sleep(0.1)  # Simula frammentazione
    
    response = sock.recv(4096)
    print("\nResponse for Chunked Transfer:\n", response.decode())
    sock.close()

def send_mixed_request():
    host = "localhost"
    port = 8080
    
    content = "X" * 4000  # 4000 caratteri
    request = ("POST /upload HTTP/1.1\r\n"
               "Host: localhost\r\n"
               "Content-Length: 4000\r\n"
               "Transfer-Encoding: chunked\r\n"
               "Connection: keep-alive\r\n\r\n")
    
    chunks = [
        "100\r\n" + "A" * 256 + "\r\n",
        "100\r\n" + "B" * 256 + "\r\n",
        "50\r\n" + "C" * 80 + "\r\n",
        "0\r\n\r\n"
    ]
    
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    sock.connect((host, port))
    sock.sendall(request.encode())
    time.sleep(0.5)
    
    for i in range(0, len(content), 512):
        sock.sendall(content[i:i+512].encode())
        time.sleep(0.1)
    
    for chunk in chunks:
        sock.sendall(chunk.encode())
        time.sleep(0.1)
    
    response = sock.recv(4096)
    print("\nResponse for Mixed Content-Length and Chunked:\n", response.decode())
    sock.close()

if __name__ == "__main__":
    send_content_length_request()
    send_chunked_request()
    send_mixed_request()
