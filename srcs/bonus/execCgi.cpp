#include "Server.hpp"
#include <sys/wait.h>
#include <signal.h>

extern volatile sig_atomic_t g_signal_status;

HttpResponse Server::execCgi(std::string const &targetPath, HttpRequest const &request) const
{
    HttpResponse response;

    int pipe_in[2], pipe_out[2];
    if (pipe(pipe_in) == -1 || pipe(pipe_out) == -1)
    {
        return genErrorPage(Location(), 500);
    }

    // Primo fork: esecuzione CGI
    pid_t pidCgi = fork();
    if (pidCgi < 0)
    {
        return genErrorPage(Location(), 500);
    }
    else if (pidCgi == 0) // Processo figlio: esegue CGI
    {
        // Redireziona stdin/stdout
        dup2(pipe_in[0], STDIN_FILENO);
        dup2(pipe_out[1], STDOUT_FILENO);

        // Chiudi i fd non utilizzati
        close(pipe_in[1]);
        close(pipe_out[0]);

        // Imposta le variabili d'ambiente per la CGI
        setenv("REQUEST_METHOD", const_cast<char *>(request.method.c_str()), 1);
        setenv("SCRIPT_FILENAME", targetPath.c_str(), 1);
        std::ostringstream oss;
        oss << request.body.size();
        setenv("CONTENT_LENGTH", oss.str().c_str(), 1);

        if (request.headers.find("Content-Type") != request.headers.end())
            setenv("CONTENT_TYPE", request.headers.find("Content-Type")->second.c_str(), 1);

        //controla estensione del file
        std::string extension = targetPath.substr(targetPath.find_last_of(".") + 1);
        char *command = NULL;
        if (extension == "py")
            command = const_cast<char *>("/usr/bin/python3");
        else
            command = const_cast<char *>("/bin/bash");
            
        
        // Esegui effettivamente lo script (qui Python) 
        char *const args[] = {
            command,
            const_cast<char *>(targetPath.c_str()),
            NULL
        };
        if (execve(args[0], args, environ) == -1)
        {
            std::cerr << RED << "[Server] Error executing CGI: " << strerror(errno) << RESET << std::endl;
            if (execve("/bin/true", (char *const []){NULL}, environ) == -1)
            {
                perror(strerror(errno));
                _exit(1);
            }

        }
        _exit(1);
    }
    else
    {
        // Processo padre: chiude i lati che non servono
        close(pipe_in[0]);  // lettura non serve
        close(pipe_out[1]); // scrittura non serve

        // --- SECONDO FORK: processo watchdog ---
        // Questo processo farà un sleep di TOT secondi,
        // e se la CGI è ancora viva, la uccide.
        pid_t pidWatch = fork();
        if (pidWatch < 0)
        {
            // Se il secondo fork fallisce, continua senza watchdog
            // (oppure kill del pidCgi, o genErrorPage...). A scelta:
            kill(pidCgi, SIGTERM);
            return genErrorPage(Location(), 500);
        }
        else if (pidWatch == 0)
        {
            // Watchdog process
            int TIMEOUT = 30;   // Tempo massimo in secondi
            int INTERVAL = 1;   // Ogni quanto controllare se la CGI è ancora viva
        
            for (int elapsed = 0; elapsed < TIMEOUT; elapsed += INTERVAL)
            {
                // Controlla se il processo CGI è ancora attivo
                if (kill(pidCgi, 0) == -1)
                {
                    // Il processo CGI è già terminato, esci subito
                    std::cerr << RED << "[Watchdog] processo CGI gia' terminato. Uscendo..." << RESET << std::endl;
                    if (execve("/bin/true", (char *const []){NULL}, environ) == -1)
                    {
                        perror(strerror(errno));
                        _exit(1);
                    }
                }
				if (g_signal_status == SIGTERM || g_signal_status == SIGINT)
				{
					kill(pidCgi, SIGTERM);
                	std::cerr << RED << "[Watchdog] ricevuto segnale di chiusura. Uscendo..." << RESET << std::endl;
					if (execve("/bin/true", (char *const []){NULL}, environ) == -1)
					{
						perror(strerror(errno));
						_exit(1);
					}
				}
            
                // Attendi un secondo prima di controllare di nuovo
                sleep(INTERVAL);
            }
        
            // Se dopo TIMEOUT secondi la CGI è ancora viva, la killiamo
            if (kill(pidCgi, 0) == 0)
            {
                kill(pidCgi, SIGTERM);
                std::cerr << RED << "[Watchdog] processo CGI terminato. Uscendo..." << RESET << std::endl;
            }
        
            // Esci senza disturbare il processo padre
            if (execve("/bin/true", (char *const []){NULL}, environ) == -1)
            {
                perror(strerror(errno));
                _exit(1);
            }
        }

        // Processo padre prosegue con l'I/O sulle pipe

        // 1) Scrivi il body su pipe_in[1] usando poll
        struct pollfd fds[2];
        fds[0].fd = pipe_in[1];
        fds[0].events = POLLOUT;

        size_t bytesWritten = 0;
        while (bytesWritten < request.body.size())
        {
            if (poll(fds, 1, -1) > 0 && (fds[0].revents & POLLOUT))
            {
                ssize_t written = write(pipe_in[1],
                                        &request.body[bytesWritten],
                                        request.body.size() - bytesWritten);
                if (written > 0)
                    bytesWritten += written;
                else
                    break;
            }
        }
        close(pipe_in[1]);

        // 2) Leggi la risposta dalla pipe_out[0]
        std::string cgiOutput;
        char buffer[1024];
        fds[0].fd = pipe_out[0];
        fds[0].events = POLLIN;

        // Esempio: poll con 5 secondi per leggere
        // (non è un "timeout" vero per la CGI, lo gestisce il watchdog).
        while (true)
        {
            int ret = poll(fds, 1, 5000);
            if (ret > 0 && (fds[0].revents & POLLIN))
            {
                ssize_t bytesRead = read(pipe_out[0], buffer, sizeof(buffer) - 1);
                if (bytesRead > 0)
                {
                    buffer[bytesRead] = '\0';
                    cgiOutput += buffer;
                }
                else
                {
                    // EOF o errore
                    break;
                }
            }
            else
            {
                // Nessun dato, esco dal loop
                break;
            }
        }
        close(pipe_out[0]);

        if (cgiOutput.empty())
        {
            // Nessun output: 204
            response.statusCode = 204;
            response.statusMessage = "No Content";
            return response;
        }

        // 3) Parse degli header e Content-Type
        size_t headerEnd = cgiOutput.find("\r\n\r\n");
        if (headerEnd != std::string::npos)
        {
            size_t ctPos = cgiOutput.find("Content-Type:");
            if (ctPos != std::string::npos)
            {
                size_t ctEnd = cgiOutput.find("\r\n", ctPos);
                if (ctEnd != std::string::npos)
                    response.contentType = cgiOutput.substr(ctPos + 13, ctEnd - (ctPos + 13));
            }
            // Rimuovi la parte di header
            cgiOutput = cgiOutput.substr(headerEnd + 4);
        }

        response.statusCode = 200;
        response.statusMessage = "OK";
        if (response.contentType.empty())
            response.contentType = "text/html";
        response.body = cgiOutput;

        return response;
    }
}
