#include "Server.hpp"

HttpResponse Server::execCgi(std::string const &targetPath, HttpRequest const &request) const
{
    HttpResponse response;
    int pipe_in[2], pipe_out[2];
    if (pipe(pipe_in) == -1 || pipe(pipe_out) == -1)
    {
        response = genErrorPage(Location(), 500, "Internal Server Error");
        return response;
    }

    pid_t pid = fork();
    if (pid < 0)
    {
        response = genErrorPage(Location(), 500, "Internal Server Error");
        return response;
    }
    else if (pid == 0) // Processo figlio (esegue CGI)
    {
        // Redirezione stdin e stdout
        dup2(pipe_in[0], STDIN_FILENO);
        dup2(pipe_out[1], STDOUT_FILENO);

        close(pipe_in[1]); // Chiude il lato di scrittura della pipe_in
        close(pipe_out[0]); // Chiude il lato di lettura della pipe_out

        // Impostiamo le variabili d'ambiente
        setenv("REQUEST_METHOD", "POST", 1);
        setenv("SCRIPT_FILENAME", targetPath.c_str(), 1);
        std::ostringstream oss;
        oss << request.body.size();
        setenv("CONTENT_LENGTH", oss.str().c_str(), 1);
        if (request.headers.find("Content-Type") != request.headers.end())
            setenv("CONTENT_TYPE", request.headers.find("Content-Type")->second.c_str(), 1);

        // Convertiamo lo scriptPath in un array di char* per `execve`
        char *const args[] = {const_cast<char *>("/usr/bin/python3.8"), const_cast<char *>(targetPath.c_str()), NULL};
        if (execve(args[0], args, environ) == -1)
            perror(strerror(errno));

        exit(1);
    }
    else // Processo padre
    {
        close(pipe_in[0]);  // Chiude il lato di lettura della pipe_in
        close(pipe_out[1]); // Chiude il lato di scrittura della pipe_out

        // Invia il body della richiesta a stdin del processo CGI
        write(pipe_in[1], &request.body[0], request.body.size());
        close(pipe_in[1]);

        // Legge la risposta dallo stdout del CGI
        std::string cgiOutput;
        char buffer[1024];
        int bytesRead;
        while ((bytesRead = read(pipe_out[0], buffer, sizeof(buffer) - 1)) > 0)
        {
            buffer[bytesRead] = '\0';
            cgiOutput += buffer;
        }
        close(pipe_out[0]);

        std::cout << RED << "Output CGI: " << cgiOutput << std::endl;

        // Attende la terminazione del processo figlio
        int status;
        waitpid(pid, &status, 0);

        // Se il processo figlio è terminato in modo anomalo
        if (WIFEXITED(status) && WEXITSTATUS(status) != 0)
        {
            response = genErrorPage(Location(), 500, "Internal Server Error");
            return response;
        }

        // controlla se la risposta è vuota restituisci 204
        if (cgiOutput.empty())
        {
            response.statusCode = 204;
            response.statusMessage = "No Content";
            return response;
        }

        // estrae il Content-Type dalla risposta CGI
        size_t headerEnd = cgiOutput.find("\r\n\r\n");
        if (headerEnd != std::string::npos)
        {
            // Trova il Content-Type
            size_t ctPos = cgiOutput.find("Content-Type:");
            if (ctPos != std::string::npos)
            {
                size_t ctEnd = cgiOutput.find("\r\n", ctPos);
                if (ctEnd != std::string::npos)
                    response.contentType = cgiOutput.substr(ctPos + 13, ctEnd - (ctPos + 13));
            }
        
            // Rimuove gli header dall'output CGI
            cgiOutput = cgiOutput.substr(headerEnd + 4);
        }

        response.statusCode = 200;
        response.statusMessage = "OK";
        response.body = cgiOutput;

        return response;
    }
}