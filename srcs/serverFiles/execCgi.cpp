#include "Server.hpp"

#include <poll.h>

HttpResponse Server::execCgi(std::string const &targetPath, HttpRequest const &request) const
{
    HttpResponse response;

    int pipe_in[2], pipe_out[2];
    
    if (pipe(pipe_in) == -1 || pipe(pipe_out) == -1)
    {
        return genErrorPage(Location(), 500, "Internal Server Error");
    }

    pid_t pid = fork();
    if (pid < 0)
    {
        return genErrorPage(Location(), 500, "Internal Server Error");
    }
    else if (pid == 0) // Processo figlio (esegue CGI)
    {
        dup2(pipe_in[0], STDIN_FILENO);
        dup2(pipe_out[1], STDOUT_FILENO);
        close(pipe_in[1]);
        close(pipe_out[0]);

        setenv("REQUEST_METHOD", "POST", 1);
        setenv("SCRIPT_FILENAME", targetPath.c_str(), 1);
        std::ostringstream oss;
        oss << request.body.size();
        setenv("CONTENT_LENGTH", oss.str().c_str(), 1);
        if (request.headers.find("Content-Type") != request.headers.end())
            setenv("CONTENT_TYPE", request.headers.find("Content-Type")->second.c_str(), 1);

        char *const args[] = {const_cast<char *>("/usr/bin/python3.8"), const_cast<char *>(targetPath.c_str()), NULL};
        if (execve(args[0], args, environ) == -1)
            perror(strerror(errno));

        exit(1);
    }
    else // Processo padre
    {
        close(pipe_in[0]);  // Chiude il lato di lettura della pipe_in
        close(pipe_out[1]); // Chiude il lato di scrittura della pipe_out

        struct pollfd fds[2];

        // **1. Scrive il body della request usando `poll()`**
        fds[0].fd = pipe_in[1];
        fds[0].events = POLLOUT;

        size_t bytesWritten = 0;
        while (bytesWritten < request.body.size())
        {
            if (poll(fds, 1, -1) > 0 && (fds[0].revents & POLLOUT))
            {
                ssize_t written = write(pipe_in[1], &request.body[bytesWritten], request.body.size() - bytesWritten);
                if (written > 0)
                    bytesWritten += written;
                else
                    break;
            }
        }
        close(pipe_in[1]);

        // **2. Legge la risposta usando `poll()`**
        std::string cgiOutput;
        char buffer[1024];

        fds[0].fd = pipe_out[0];
        fds[0].events = POLLIN;

        while (true)
        {
            if (poll(fds, 1, 5000) > 0 && (fds[0].revents & POLLIN))
            {
                ssize_t bytesRead = read(pipe_out[0], buffer, sizeof(buffer) - 1);
                if (bytesRead > 0)
                {
                    buffer[bytesRead] = '\0';
                    cgiOutput += buffer;
                }
                else
                    break;
            }
            else
                break;
        }
        close(pipe_out[0]);

        // Attende la terminazione del processo figlio
        int status;
        waitpid(pid, &status, 0);
        if (WIFEXITED(status) && WEXITSTATUS(status) != 0)
        {
            return genErrorPage(Location(), 500, "Internal Server Error");
        }

        // **3. Estrae il Content-Type e rimuove gli header**
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
