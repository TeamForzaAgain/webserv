#include "Server.hpp"

int main()
{
	Server server(AF_INET, SOCK_STREAM, 0, 8080, INADDR_ANY);
	server.start();
}