#include "ServerManager.hpp"

ClientSocket::ClientSocket(int fd, Server *server) : Socket(fd),  _server(server),
        _status(1), _lastActivity(time(NULL)), _keepAlive(false), _contentLength(0), _chunkLength(-1), _headersLenght(0),
        _toUpload(false)
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

time_t ClientSocket::getLastActivity() const
{
    return _lastActivity;
}

void ClientSocket::addBuffer(const char *buffer, int bytesRead)
{
    _buffer.insert(_buffer.end(), buffer, buffer + bytesRead);
}

void ClientSocket::setLastActivity()
{
    _lastActivity = time(NULL);
}


void ClientSocket::genResponse(ServerManager &serverManager)
{
    _status = parseRequest(serverManager);

    std::cout << "[ClientSocket] Request status: " << _status << std::endl;

    if (_status > 0)
    {
        _response = _server->genResponse(_request, _status);
        _status = 1;
        _buffer.clear();
        _request.clear();
    }
    else if (_status == -1)
        _request.clear();
    else
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
