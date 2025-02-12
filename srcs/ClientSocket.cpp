#include "ServerManager.hpp"

ClientSocket::ClientSocket(int fd, Server const *server) : Socket(fd),  _server(server),
        _status(1), _keepAlive(false), _contentLength(0), _chunkLength(-1), _headersLenght(0),
        _isMultipart(false)
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

int ClientSocket::getStatus() const
{
    return _status;
}

void ClientSocket::addBuffer(const char *buffer, int bytesRead)
{
    _buffer.insert(_buffer.end(), buffer, buffer + bytesRead);
}

int ClientSocket::parseRequest(ServerManager &serverManager)
{
    std::vector<char> requestBuffer(_buffer.begin(), _buffer.end());
    
    if (_status == 1 || _status == 2) // Nuova richiesta, dobbiamo parsare gli header
    {
        if (_status == 2)
            _status = 1;
        _request.clear();
        _chunkLength = -1;
        _headersLenght = 0;
        _contentLength = 0;
        _bytesWritten = 0;
        _isMultipart = false;
        
        std::string requestStr(requestBuffer.begin(), requestBuffer.end());
        size_t headerEnd = requestStr.find("\r\n\r\n");
        
        if (headerEnd == std::string::npos) // Header incompleto
            return 2;
        
        _headersLenght = headerEnd + 4;
        std::stringstream stream(requestStr.substr(0, headerEnd));
        std::string line;
        
        // Parsiamo la prima riga (metodo, path, versione)
        if (!std::getline(stream, line) || line.find(" ") == std::string::npos)
            return -1;
        std::istringstream firstLine(line);
        if (!(firstLine >> _request.method >> _request.path))
            return -1;
        
        // Parsiamo gli header
        while (std::getline(stream, line) && !line.empty())
        {
            size_t sep = line.find(": ");
            if (sep == std::string::npos)
                return -1;
            _request.headers[line.substr(0, sep)] = line.substr(sep + 2);
        }
        
        // Controlliamo Content-Length o Transfer-Encoding
        std::map<std::string, std::string>::iterator it;
        it = _request.headers.find("Content-Length");
        if (it != _request.headers.end())
        {
            _contentLength = atoi(it->second.c_str());
        }
        else if ((it = _request.headers.find("Transfer-Encoding")) != _request.headers.end())
            _chunkLength = 0; // Attiviamo la modalità chunked
        else if (_request.method == "POST") // POST deve avere un body definito
            return -1;

        // Controllo se la richiesta è multipart/form-data
        it = _request.headers.find("Content-Type");
        if (it != _request.headers.end() && it->second.find("multipart/form-data") != std::string::npos)
        {
            _isMultipart = true;
            size_t boundaryPos = it->second.find("boundary=");
            if (boundaryPos != std::string::npos)
                _boundary = "--" + it->second.substr(boundaryPos + 9);
        }

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
    }
    
    // Lettura del body
    std::vector<char> bodyPart;
    if (_status == 1)
        bodyPart.assign(requestBuffer.begin() + _headersLenght, requestBuffer.end());
    else
        bodyPart.assign(requestBuffer.begin(), requestBuffer.end());
    
    if (_isMultipart)
    {
        if (!_uploadFile.is_open())
        {
            std::string bodyStr(bodyPart.begin(), bodyPart.end());
            size_t filenamePos = bodyStr.find("filename=");
            std::string filename = "uploaded_file.bin";
            if (filenamePos != std::string::npos)
            {
                filenamePos += 10; // Salta "filename="
                size_t filenameEnd = bodyStr.find('"', filenamePos);
                if (filenameEnd != std::string::npos)
                    filename = bodyStr.substr(filenamePos, filenameEnd - filenamePos);
            }
            _request.headers["filename"] = filename;

            Location const *location = _server->getUploadLocation();
            if (!location)
                return -1;
            if (_request.path.find(location->path) != 0)
                _isMultipart = false;
            else
            {
                std::string filePath = _server->buildFilePath(_request.path, *location) + filename;
                _uploadFile.open(filePath.c_str(), std::ios::binary);
                if (!_uploadFile)
                {
                    perror(strerror(errno));
                    return -1;
                }
            }
        } 
    }

    if (_contentLength > 0)
    {
        if (_isMultipart)
        {
            if (_bytesWritten == 0)
            {
                std::vector<char>::iterator it = std::search(bodyPart.begin(), bodyPart.end(), "\r\n\r\n", "\r\n\r\n" + 4);
                if (it != bodyPart.end())
                {
                    _bytesWritten += it - bodyPart.begin() + 4;
                    bodyPart.erase(bodyPart.begin(), it + 4);
                }
            }
            _uploadFile.write(bodyPart.data(), bodyPart.size());
            _bytesWritten += bodyPart.size();
        }
        else
            _request.body.insert(_request.body.end(), bodyPart.begin(), bodyPart.end());
        std::cout << ORANGE << _request.body.size() << " / " << _contentLength << RESET << std::endl;

        if (_isMultipart && _bytesWritten >= _contentLength)
        {
            _uploadFile.close();
            return 1; // Completa
        }
        else if ((int)_request.body.size() >= _contentLength)
            return 1; // Completa
        else 
            return 0; // Incompleta
    }
    else if (_chunkLength != -1) // Gestione chunked
    {
        while (!bodyPart.empty())
        {
            if (_chunkLength == 0) // Necessario leggere il chunk size
            {
                size_t chunkSizeEnd = 0;
                while (chunkSizeEnd < bodyPart.size() && bodyPart[chunkSizeEnd] != '\r')
                    ++chunkSizeEnd;
                if (chunkSizeEnd == bodyPart.size() || chunkSizeEnd + 1 >= bodyPart.size() || bodyPart[chunkSizeEnd + 1] != '\n')
                    return 0; // Chunk incompleto
                
                std::string chunkSizeStr(bodyPart.begin(), bodyPart.begin() + chunkSizeEnd);
                _chunkLength = strtol(chunkSizeStr.c_str(), NULL, 16);
                bodyPart.erase(bodyPart.begin(), bodyPart.begin() + chunkSizeEnd + 2);
                
                if (_chunkLength == 0) // Chunk finale trovato
                    return 1;
            }
            
            size_t chunkToRead = std::min((size_t)_chunkLength, bodyPart.size());
            if (_isMultipart)
            {
                if (_bytesWritten == 0)
                {
                    std::vector<char>::iterator it = std::search(bodyPart.begin(), bodyPart.end(), "\r\n\r\n", "\r\n\r\n" + 4);
                    if (it != bodyPart.end())
                    {
                        _bytesWritten += it - bodyPart.begin() + 4;
                        bodyPart.erase(bodyPart.begin(), it + 4);
                    }
                }
                _uploadFile.write(bodyPart.data(), chunkToRead);
                _bytesWritten += chunkToRead;
            }
            else
                _request.body.insert(_request.body.end(), bodyPart.begin(), bodyPart.begin() + chunkToRead);
            bodyPart.erase(bodyPart.begin(), bodyPart.begin() + chunkToRead);
            _chunkLength -= chunkToRead;
            
            if (_chunkLength == 0 && bodyPart.size() >= 2) // Se il chunk è finito e c'è "\r\n", lo rimuoviamo
            {
                bodyPart.erase(bodyPart.begin(), bodyPart.begin() + 2);
            }
            else if (_chunkLength > 0) // Chunk parziale, attendiamo altro input
            {
                return 0;
            }
        }
        return 0;
    }
    
    return _status;
}


void ClientSocket::genResponse(ServerManager &serverManager)
{
    _status = parseRequest(serverManager);

    // Stampa della struttura HttpRequest
    std::cout << ORANGE << "Metodo: " << _request.method << RESET << std::endl;
    std::cout << ORANGE << "Path: " << _request.path << RESET << std::endl;
    for (std::map<std::string, std::string>::iterator it = _request.headers.begin(); it != _request.headers.end(); ++it)
    {
        std::cout << ORANGE << it->first << ": " << it->second << RESET << std::endl;
    }
    std::cout << ORANGE << "Body: ";
    for (size_t i = 0; i < _request.body.size(); i++)
        std::cout << _request.body[i];
    std::cout << RESET << std::endl;
    std::cout << ORANGE << "Request status: " << _status << RESET << std::endl;

    if (_status != 2)
        _buffer.clear();
    if (_status != 0)
    {
        if (_status != 2)
            _response = _server->genResponse(_request);
        _request.clear();
    }
}

std::string ClientSocket::getBuffer() const
{
    return std::string(_buffer.begin(), _buffer.end());
}

std::string ClientSocket::getResponse() const
{
	return _response;
}
