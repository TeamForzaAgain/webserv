#include "ClientSocket.hpp"

ClientSocket::ClientSocket(int fd) : _fd(fd), _requestComplete(false)
{}

ClientSocket::~ClientSocket()
{
	close(_fd);
}

int ClientSocket::getFd() const
{
	return _fd;
}

void ClientSocket::addBuffer(const char *buffer)
{
	_buffer.insert(_buffer.end(), buffer, buffer + strlen(buffer));
}

static int stringToInt(const std::string &str)
{
    std::istringstream iss(str);
    int result;
    iss >> result;

    if (iss.fail()) {
        // Gestisci errori di conversione
        throw std::runtime_error("Errore: stringa non convertibile in intero");
    }

    return result;
}

int ClientSocket::parseMessage()
{
	std::string buffer = std::string(_buffer.begin(), _buffer.end());
    // Cerca la fine degli header HTTP: \r\n\r\n
    size_t headerEnd = buffer.find("\r\n\r\n");
    if (headerEnd == std::string::npos) {
        return 0; // Header incompleto
    }

    // Verifica la presenza dell'header Content-Length
    size_t contentLengthPos = buffer.find("Content-Length: ");
    if (contentLengthPos != std::string::npos) {
        // Trova il valore di Content-Length
        contentLengthPos += 16; // Salta "Content-Length: "
        size_t contentLengthEnd = buffer.find("\r\n", contentLengthPos);
        if (contentLengthEnd == std::string::npos) {
            return -1; // Header malformato
        }

     int contentLength = stringToInt(buffer.substr(contentLengthPos, contentLengthEnd - contentLengthPos));

        // Calcola la posizione di inizio del corpo del messaggio
        size_t bodyStart = headerEnd + 4; // Dopo \r\n\r\n
        size_t bodyLength = buffer.size() - bodyStart;

        // Controlla se il corpo del messaggio è completo
        if (bodyLength >= (size_t)contentLength) {
            return 1; // Messaggio completo
        } else {
            return 0; // Corpo incompleto
        }
    }

    // Nessun Content-Length: considera il messaggio completo se ha solo header
    return 1;
}