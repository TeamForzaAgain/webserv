#include "ServerManager.hpp"

ClientSocket::ClientSocket(int fd, Server const *server) : _fd(fd), _server(server)
{
	_response = "ClientSocket response " + std::to_string(fd);
    _keepAlive = false;
}

ClientSocket::~ClientSocket()
{
	close(_fd);
}

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

void ClientSocket::parseHostConnection(ServerManager &serverManager, const std::string &request)
{
    size_t hostPos = request.find("Host: ");
    if (hostPos != std::string::npos)
    {
        hostPos += 6; // Saltiamo "Host: "
        size_t hostEnd = request.find("\r\n", hostPos);
        if (hostEnd != std::string::npos)
        {
            std::string host = request.substr(hostPos, hostEnd - hostPos);
            std::cout << "Parsed Host: " << host << std::endl;

            // Trova il nuovo server in base all'host e alla ListeningSocket del server attuale
            Server const *newServer = serverManager.findServerByHost(host, _server);
            if (newServer)
            {
                std::cout << "Switching server based on Host: " << host << std::endl;
                _server = newServer;
            }
        }
    }

    // Controllo dell'header "Connection:"
    size_t connPos = request.find("Connection: ");
    if (connPos != std::string::npos)
    {
        connPos += 11; // Saltiamo "Connection: "
        size_t connEnd = request.find("\r\n", connPos);
        if (connEnd != std::string::npos)
        {
            std::string connection = request.substr(connPos, connEnd - connPos);
            std::cout << "Parsed Connection: " << connection << std::endl;

            // Gestiamo il valore di Connection
            if (connection == "keep-alive")
                _keepAlive = true;
            else if (connection == "close")
                _keepAlive = false;
        }
    }
}

void ClientSocket::genResponse(ServerManager &serverManager)
{
    std::string const &request = std::string(_buffer.begin(), _buffer.end());

    parseHostConnection(serverManager, request);
    _response = _server->genResponse(request);
    _buffer.clear();
}

std::string ClientSocket::getBuffer() const
{
    return std::string(_buffer.begin(), _buffer.end());
}

std::string ClientSocket::getResponse() const
{
	return _response;
}
