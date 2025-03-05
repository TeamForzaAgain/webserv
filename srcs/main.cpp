#include "Include.hpp"
#include "SyntaxValidator.hpp"
#include "LogicValidator.hpp"
#include "ServerManager.hpp"

volatile sig_atomic_t g_signal_status = 0;


void handleSigChld()
{
	while (waitpid(-1, NULL, WNOHANG) > 0);
}

void signalHandler(int signum)
{
	g_signal_status = signum;
	std::cerr << BLUE << "\n[SignalHandler] Intercettato segnale (" << signum << ")." << RESET << std::endl;

	switch (signum)
	{
	case SIGCHLD:
		handleSigChld();
		break;

	case SIGINT:
	case SIGTERM:
	case SIGHUP:
		std::cerr << BLUE << "[SignalHandler] Arresto del server in corso..." << RESET << std::endl;
		break;

	default:
		std::cerr << BLUE << "[SignalHandler] Segnale non gestito: " << signum << RESET << std::endl;
		break;
	}
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

void initializeServers(const std::vector<ServerConfig>& servers, ServerManager& serverManager)
{
	for (size_t i = 0; i < servers.size(); i++)
	{
		for (size_t j = 0; j < servers[i].listens.size(); j++)
		{
			serverManager.newServer(
				AF_INET, 
				SOCK_STREAM, 
				0, 
				servers[i].listens[j].port, 
				servers[i].listens[j].ip, 
				servers[i]
			);
		}
	}
}

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

		validateServers(parsedServers);

		initializeServers(parsedServers, serverManager);

		serverManager.run();

	}
	catch (std::exception &e)
	{
		std::cerr << RED << "[ERROR] " << e.what() << RESET << std::endl;
		return (1);
	}
	return (0);
}
