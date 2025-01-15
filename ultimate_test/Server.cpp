#include "Server.hpp"

Server::Server(ListeningSocket *ls, std::string const &serverName) : _ls(ls), _serverName(serverName)
{}

Server::~Server()
{}

bool Server::operator<(const Server &other) const
{
    // Puoi decidere come ordinare i server. Ecco un esempio basato sul nome del server:
    return _serverName < other._serverName;
}
