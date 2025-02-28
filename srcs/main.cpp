#include "Include.hpp"
#include "SyntaxValidator.hpp"
#include "LogicValidator.hpp"
#include "ServerManager.hpp"

volatile sig_atomic_t g_signal_status = 0;

// void signalHandler(int signum)
// {
// 	std::cout << MAGENTA << "\nIntercettato il segnale (" << signum << "). Pulizia e uscita..." << RESET << std::endl;
// 	if (signum == SIGCHLD)
// 	{
// 		while (waitpid(-1, NULL, WNOHANG) > 0)
// 			;
// 	}
// }

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

static bool preliminaryChecks(int ac, char **av, std::stringstream &buffer)
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
			} else
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
				} else
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
						} else
						{
							listenConfig.ip = INADDR_ANY;
							listenConfig.port = atoi(value.c_str());
						}
						server.listens.push_back(listenConfig);
					}
				}
			}
		}
		servers.push_back(server);
	}
	return servers;
}

void handleSigChld()
{
    while (waitpid(-1, NULL, WNOHANG) > 0);
}

void signalHandler(int signum)
{
	g_signal_status = signum;
	std::cerr << "\n[SignalHandler] Intercettato segnale (" << signum << ")." << std::endl;

	switch (signum)
	{
	case SIGCHLD:
		handleSigChld();
		break;

	case SIGINT:
	case SIGTERM:
	case SIGHUP:
		std::cerr << "[SignalHandler] Arresto del server in corso..." << std::endl;

		exit(EXIT_SUCCESS);
		break;

	default:
		std::cerr << "[SignalHandler] Segnale non gestito: " << signum << std::endl;
		break;
	}
}

void setupSignalHandlers()
{
	struct sigaction sa;
	sa.sa_handler = signalHandler;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_RESTART;

	sigaction(SIGINT, &sa, NULL);
	sigaction(SIGTERM, &sa, NULL);
	sigaction(SIGHUP, &sa, NULL);
	sigaction(SIGCHLD, &sa, NULL);
}

/*
 * Gestore generale dei segnali intercettati.
 *
 * Segnali gestiti:
 * - SIGCHLD : Indica la terminazione di un processo figlio.
 * - SIGINT  : Generato da Ctrl+C, indica la richiesta di terminazione del server.
 * - SIGTERM : Richiesta di terminazione gentile del server (es. `kill` di default).
 * - SIGHUP  : Segnale di "hangup" (disconnessione terminale), spesso usato per ricaricare la configurazione.
 *
 * In caso di SIGINT, SIGTERM o SIGHUP, il server viene chiuso in modo controllato.
 */

int main(int ac, char **av)
{
	std::stringstream fileStream;

	setupSignalHandlers();

	try
	{
		preliminaryChecks(ac, av, fileStream);

		SyntaxValidator validator(fileStream);
		validator.validate();
		fileStream.clear();
		fileStream.seekg(0, std::ios::beg);

		LogicValidator logicValidator(fileStream);
		logicValidator.validate();
		fileStream.clear();
		fileStream.seekg(0, std::ios::beg);

		std::cout << GREEN << "[SUCCESS] File di configurazione validato con successo!" << RESET << std::endl;

		std::stringstream cleanedConfig;
		cleanConfig(fileStream, cleanedConfig);

		std::vector<std::string> servers = splitServers(cleanedConfig);

		std::vector<ServerConfig> parsedServers = parseServers(servers);

		ServerManager serverManager;

		for (size_t i = 0; i < parsedServers.size(); i++)
		{
			for (size_t j = 0; j < parsedServers[i].listens.size(); j++)
			{
	   		serverManager.newServer(
			AF_INET, 
			SOCK_STREAM, 
			0, 
			parsedServers[i].listens[j].port, 
			parsedServers[i].listens[j].ip, 
			parsedServers[i]
		);
	}
}

serverManager.run();

	}
	catch (std::exception &e)
	{
		std::cerr << RED << "[ERROR] " << e.what() << RESET << std::endl;
		return (1);
	}
	return (0);
}
