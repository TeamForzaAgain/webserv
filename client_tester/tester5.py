import socket
import time

def send_binary_content_length():
    host = "localhost"
    port = 8080
    
    with open("test_binary.bin", "wb") as f:
        f.write(bytes(range(256)))  # Scriviamo un file binario con tutti i byte possibili
    
    with open("test_binary.bin", "rb") as f:
        binary_data = f.read()
    
    request = ("POST /upload HTTP/1.1\r\n"
               "Host: localhost\r\n"
               "Content-Length: {}\r\n"
               "Connection: keep-alive\r\n\r\n".format(len(binary_data))).encode()
    
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    sock.connect((host, port))
    sock.sendall(request)
    time.sleep(0.1)
    sock.sendall(binary_data)
    
    response = sock.recv(4096)
    print("\nResponse for Binary Content-Length:\n", response.decode(errors='ignore'))
    sock.close()

def send_binary_chunked():
    host = "localhost"
    port = 8080
    
    with open("test_binary.bin", "rb") as f:
        binary_data = f.read()
    
    request = ("POST /upload HTTP/1.1\r\n"
               "Host: localhost\r\n"
               "Transfer-Encoding: chunked\r\n"
               "Connection: keep-alive\r\n\r\n").encode()
    
    chunks = [binary_data[i:i+64] for i in range(0, len(binary_data), 64)]
    
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    sock.connect((host, port))
    sock.sendall(request)
    time.sleep(0.1)
    
    for chunk in chunks:
        chunk_size = hex(len(chunk))[2:].encode() + b"\r\n"
        sock.sendall(chunk_size)
        sock.sendall(chunk)
        sock.sendall(b"\r\n")
        time.sleep(0.05)
    
    sock.sendall(b"0\r\n\r\n")  # Terminazione chunked
    
    response = sock.recv(4096)
    print("\nResponse for Binary Chunked Transfer:\n", response.decode(errors='ignore'))
    sock.close()

if __name__ == "__main__":
    send_binary_content_length()
    send_binary_chunked()
