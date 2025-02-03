#include "ServerManager.hpp"

ClientSocket::ClientSocket(int fd, Server const *server) : Socket(fd), _server(server), _keepAlive(false)
{}

ClientSocket::~ClientSocket()
{}

int ClientSocket::getFd() const
{
	return _fd;
}

bool ClientSocket::getKeepAlive() const
{
    return _keepAlive;
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

int ClientSocket::parseEndMessage()
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

int ClientSocket::genResponse(ServerManager &serverManager)
{
    std::string const &requestString = std::string(_buffer.begin(), _buffer.end());
    // Parsing della richiesta, creazione della struttura HttpRequest
    int requestStatus = _request.fromString(requestString);

    // Stampa della struttura HttpRequest
    std::cout << ORANGE << "Metodo: " << _request.method << RESET << std::endl;
    std::cout << ORANGE << "Path: " << _request.path << RESET << std::endl;
    for (std::map<std::string, std::string>::iterator it = _request.headers.begin(); it != _request.headers.end(); ++it)
    {
        std::cout << ORANGE << it->first << ": " << it->second << RESET << std::endl;
    }
    std::cout << ORANGE << "Body: " << _request.body << RESET << std::endl;
    std::cout << ORANGE << "Request status: " << requestStatus << RESET << std::endl;


    std::map<std::string, std::string>::iterator it;
    it = _request.headers.find("Host");
    if (it != _request.headers.end() && !it->second.empty())
    {
        Server const *newServer = serverManager.findServerByHost(it->second, _server);
        if (newServer)
            _server = newServer;
    }

    it = _request.headers.find("Connection");
    if (it != _request.headers.end() && !it->second.empty())
    {
        if (it->second == "keep-alive")
            _keepAlive = true;
        else if (it->second == "close")
            _keepAlive = false;
    }
    if (requestStatus == 1)
    {
        _response = _server->genResponse(requestString);
        _buffer.clear();
        _request.clear();
    }
    return requestStatus;
}

std::string ClientSocket::getBuffer() const
{
    return std::string(_buffer.begin(), _buffer.end());
}

std::string ClientSocket::getResponse() const
{
	return _response;
}
