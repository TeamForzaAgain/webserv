#include "Server.hpp"

Server::Server(ListeningSocket *ls, std::string const &serverName) : _ls(ls), _serverName(serverName)
{}

Server::~Server()
{}


