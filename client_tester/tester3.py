import socket
import time

def send_content_length_request():
    host = "localhost"
    port = 8080
    
    content = "A" * 150  # 150 caratteri, più grande di BUFFERSIZE=100
    request = ("POST /upload HTTP/1.1\r\n"
               "Host: localhost\r\n"
               "Content-Length: 150\r\n"
               "Connection: keep-alive\r\n\r\n")
    
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    sock.connect((host, port))
    sock.sendall(request.encode())
    time.sleep(1)  # Simula una trasmissione parziale
    sock.sendall(content[:75].encode())  # Primo pezzo (75 byte)
    time.sleep(1)
    sock.sendall(content[75:].encode())  # Secondo pezzo (75 byte)
    
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
    
    chunks = ["C8\r\n" + "A" * 200 + "\r\n",  # 80 byte
              "C8\r\n" + "B" * 200 + "\r\n",  # 80 byte
              "0\r\n\r\n"]  # Terminazione
    
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    sock.connect((host, port))
    sock.sendall(request.encode())
    time.sleep(1)
    
    for chunk in chunks:
        sock.sendall(chunk.encode())
        time.sleep(1)
    
    response = sock.recv(4096)
    print("\nResponse for Chunked Transfer:\n", response.decode())
    sock.close()

if __name__ == "__main__":
    send_content_length_request()
    send_chunked_request()
