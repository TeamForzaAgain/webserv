#include "Include.hpp"

static bool hasConfExtension(const std::string &filename)
{
	if (filename.size() > 5 && filename.substr(filename.size() - 5) == ".conf")
		return (true);

	throw std::runtime_error("Il file deve avere estensione .conf");
}

static bool fileExistsAndReadable(const std::string &filename)
{
	struct stat buffer;
	std::ifstream file;

	if (stat(filename.c_str(), &buffer) != 0)
	{
		throw std::runtime_error("Il file non esiste");
	}

	if (S_ISDIR(buffer.st_mode))
	{
		throw std::runtime_error("Il percorso è una directory");
	}

	file.open(filename.c_str());
	if (!file.is_open())
	{
		throw std::runtime_error("Impossibile aprire il file");
	}
	return (true);
}

static bool loadFileToStream(const std::string &filename, std::stringstream &buffer)
{
	std::ifstream file(filename.c_str());

	if (!file.is_open())
	{
		throw std::runtime_error("Impossibile aprire il file");
	}
	buffer << file.rdbuf();
	file.close();
	return (true);
}

bool preliminaryChecks(int ac, char **av, std::stringstream &buffer)
{
	std::string filename;

	if (ac != 2)
	{
		std::ostringstream oss;
		oss << "Uso corretto: " << av[0] << " <file_di_configurazione.conf>" << std::endl;
		throw std::runtime_error(oss.str());
	}

	filename = av[1];

	hasConfExtension(filename);

	fileExistsAndReadable(filename);

	loadFileToStream(filename, buffer);

	std::cout << GREEN << "[SUCCESS] Il file è stato caricato correttamente!" << RESET << std::endl;
	return (true);
}

void cleanConfig(std::istream &configStream, std::stringstream &cleanedConfig)
{
	std::string line;

	while (std::getline(configStream, line))
	{
		std::string temp;
		for (size_t i = 0; i < line.size(); i++)
		{
			if (line[i] != ';') temp += line[i];
		}
		line = temp;

		size_t start = 0;
		while (start < line.size() && (line[start] == ' ' || line[start] == '\t'))
			start++;

		size_t end = line.size();
		while (end > start && (line[end - 1] == ' ' || line[end - 1] == '\t'))
			end--;

		line = line.substr(start, end - start);

		if (!line.empty())
		{
			cleanedConfig << line << "\n";
		}
	}
}

std::vector<std::string> splitServers(std::istream &configStream)
{
	std::vector<std::string> servers;
	std::stringstream currentServer;
	std::string line;
	bool insideServer = false;
	int bracketCount = 0;

	while (std::getline(configStream, line))
	{
		if (line.find("server {") != std::string::npos)
		{
			if (insideServer)
			{
				servers.push_back(currentServer.str());
				currentServer.str("");
				currentServer.clear();
			}
			insideServer = true;
			bracketCount = 1;
		} else if (insideServer)
		{
			if (line.find("{") != std::string::npos)
			{
				bracketCount++;
			}
			if (line.find("}") != std::string::npos)
			{
				bracketCount--;
			}
			if (bracketCount == 0) {  
				servers.push_back(currentServer.str());
				currentServer.str("");
				currentServer.clear();
				insideServer = false;
				continue;
			}
		}
		if (insideServer)
		{
			currentServer << line << "\n";
		}
	}
	return servers;
}



std::vector<ServerConfig> parseServers(const std::vector<std::string> &serverBlocks)
{
	std::vector<ServerConfig> servers;

	for (size_t i = 0; i < serverBlocks.size(); i++)
	{
		ServerConfig server;
		std::istringstream serverStream(serverBlocks[i]);
		std::string line;
		bool insideLocation = false;
		Location currentLocation;

		while (std::getline(serverStream, line))
		{
			if (line.find("location ") != std::string::npos)
			{
				if (insideLocation)
				{
					server.locations.push_back(currentLocation);
				}
				currentLocation = Location();
				insideLocation = true;
				std::istringstream iss(line);
				std::string keyword;
				iss >> keyword >> currentLocation.path;
				continue;
			}

			if (line.find("}") != std::string::npos && insideLocation)
			{
				server.locations.push_back(currentLocation);
				insideLocation = false;
				continue;
			}

			std::istringstream iss(line);
			std::string key, value;
			iss >> key;

			if (key == "methods")
			{
				while (iss >> value)
				{
					if (value == "GET") currentLocation.allowedMethods.GET = true;
					else if (value == "POST") currentLocation.allowedMethods.POST = true;
					else if (value == "DELETE") currentLocation.allowedMethods.DELETE = true;
				}
			} 
			else
			{
				iss >> value;
				if (insideLocation)
				{
					if (key == "root") currentLocation.root = value;
					else if (key == "auto_index") currentLocation.autoIndex = (value == "on");
					else if (key == "upload") currentLocation.upload = (value == "on");
					else if (key == "cgi") currentLocation.cgi = (value == "on");
					else if (key == "alias") currentLocation.isAlias = (value == "on");
					else if (key == "index") currentLocation.indexFiles.push_back(value);
					else if (key == "return")
					{
						int returnCode;
						std::string returnUrl;
						returnCode = atoi(value.c_str());
						iss >> returnUrl;
						currentLocation.returnConfig.code = returnCode;
						currentLocation.returnConfig.url = returnUrl;
					}
				} 
				else
				{
					if (key == "server_name") server.hostName = value;
					else if (key == "max_body_size") server.maxBodySize = static_cast<int>(strtol(value.c_str(), NULL, 10));
					else if (key == "root") server.defLocation.root = value;
					else if (key == "error_page")
					{
						int errorCode = atoi(value.c_str());
						std::string errorFile;
						iss >> errorFile;
						server.defLocation.errorPages[errorCode] = errorFile;
					}
					else if (key == "listen")
					{
						ListenConfig listenConfig;
						size_t colonPos = value.find(":");
						if (colonPos != std::string::npos)
						{
							std::string ipString = value.substr(0, colonPos);
							std::string portString = value.substr(colonPos + 1);
							if (ipString == "localhost")
								listenConfig.ip = htonl(INADDR_LOOPBACK);
							else
								listenConfig.ip = inet_addr(ipString.c_str());
							listenConfig.port = atoi(value.substr(colonPos + 1).c_str());
						} 
						else
						{
							listenConfig.ip = INADDR_ANY;
							listenConfig.port = atoi(value.c_str());
						}
						server.listens.push_back(listenConfig);
					}
					else if (key == "return")
					{
						int returnCode;
						std::string returnUrl;
						returnCode = atoi(value.c_str());
						iss >> returnUrl;
						server.defaultReturn.code = returnCode;
						server.defaultReturn.url = returnUrl;
					}
				}
			}
		}
		servers.push_back(server);
	}
	return servers;
}


void checkDuplicateLocations(const std::vector<Location>& locations)
{
	std::set<std::string> locationSet;
	for (size_t i = 0; i < locations.size(); ++i)
	{
		if (!locationSet.insert(locations[i].path).second)
		{
			std::cerr << RED << "Errore: Location duplicata trovata: " << locations[i].path << std::endl;
			throw std::runtime_error("Location duplicata trovata.");
		}
	}
}

void checkListenDirective(const ServerConfig& server)
{
	if (server.listens.empty())
	{
		std::cerr << RED << "Errore: Il server " << server.hostName
					<< " non ha alcuna direttiva 'listen' e sarà irraggiungibile." << RESET << std::endl;
		throw std::runtime_error("Nessuna direttiva 'listen' definita per il server.");
	}
}

void checkRootDirectory(const ServerConfig& server)
{
	if (server.defLocation.root.empty())
	{
		std::cerr << RED << "Errore: Il server " << server.hostName
					<< " non ha una root directory definita." << RESET << std::endl;
		throw std::runtime_error("Root directory non definita per il server.");
	}
}

void checkLocationValidity(const ServerConfig& server, const Location& loc)
{
	if (!loc.allowedMethods.GET && !loc.allowedMethods.POST && !loc.allowedMethods.DELETE)
	{
		std::cerr << RED << "Errore: La location " << loc.path
					<< " del server " << server.hostName
					<< " non permette alcun metodo HTTP valido." << RESET << std::endl;
		throw std::runtime_error("Nessun metodo HTTP valido definito per la location.");
	}
}

void checkLocations(const ServerConfig& server)
{
	for (size_t j = 0; j < server.locations.size(); j++)
	{
		checkLocationValidity(server, server.locations[j]);
	}
}

void checkDuplicateListen(const std::vector<ServerConfig>& servers)
{
	std::set<std::string> listenSet;
	std::set<int> wildcardPorts;

	for (size_t i = 0; i < servers.size(); i++)
	{
		std::set<std::string> localListenSet;
		std::set<int> localWildcardPorts;

		for (size_t j = 0; j < servers[i].listens.size(); j++)
		{
			std::string ipStr = inet_ntoa(*(struct in_addr*)&servers[i].listens[j].ip);
			int port = servers[i].listens[j].port;

			std::ostringstream portStr;
			portStr << port;
			std::string portKey = portStr.str();

			std::ostringstream oss;
			oss << ipStr << ":" << port;
			std::string listenKey = oss.str();

			if (localWildcardPorts.count(port))
			{
				std::cerr << RED << "[ERRORE] Conflitto: il server tenta di ascoltare su " << listenKey
						  << " ma la porta " << port << " è già occupata da una wildcard interna (0.0.0.0)." 
						  << RESET << std::endl;
				throw std::runtime_error("Conflitto di listen interno al server.");
			}

			if (!localListenSet.insert(listenKey).second)
			{
				std::cerr << RED << "[ERRORE] La porta " << port
						  << " sull'indirizzo " << ipStr
						  << " è già usata da un'altra location dello stesso server!" << RESET << std::endl;
				throw std::runtime_error("Conflitto di listen interno al server.");
			}

			if (ipStr == "0.0.0.0")
			{
				localWildcardPorts.insert(port);

				for (std::set<std::string>::iterator it = localListenSet.begin(); it != localListenSet.end(); ++it)
				{
					if (it->find(":" + portKey) != std::string::npos)
					{
						std::cerr << RED << "[ERRORE] Conflitto: il server tenta di ascoltare su 0.0.0.0:" << port
								  << " ma questa porta è già usata da un IP specifico all'interno dello stesso server!" << RESET << std::endl;
						throw std::runtime_error("Conflitto di listen interno al server.");
					}
				}
			}
		}
	}
}

void validateServers(const std::vector<ServerConfig>& servers)
{
	checkDuplicateListen(servers);

	for (size_t i = 0; i < servers.size(); i++)
	{
		checkListenDirective(servers[i]);
		checkRootDirectory(servers[i]);
		checkDuplicateLocations(servers[i].locations);
		checkLocations(servers[i]);
	}
}
