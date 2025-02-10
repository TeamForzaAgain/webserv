#include "Include.hpp"

/* int HttpRequest::fromString(const std::string &request)
{
    std::istringstream requestStream(request);
    std::string line;

    // Controllo se gli header sono completi
    size_t headerEnd = request.find("\r\n\r\n");
    if (headerEnd == std::string::npos)
        return 0; // **Header incompleto, attendiamo altri dati

    // Parsing della prima riga (Metodo, Path, HTTP Version)
    if (method.empty() && std::getline(requestStream, line))
    {
        std::istringstream lineStream(line);
        lineStream >> method >> path; // Ignoriamo la versione HTTP

        // Convertiamo il metodo in maiuscolo per uniformità
        for (size_t i = 0; i < method.size(); i++)
            method[i] = toupper(method[i]);
    }

    // Parsing degli Header (solo se non sono già stati letti)
    if (headers.empty())
    {
        while (std::getline(requestStream, line) && line != "\r")
        {
            size_t separator = line.find(": ");
            if (separator != std::string::npos)
            {
                std::string key = line.substr(0, separator);
                std::string value = line.substr(separator + 2);
                headers[key] = value;
            }
        }
    }

    if (headers.find("Transfer-Encoding") != headers.end() &&
        headers["Transfer-Encoding"].find("chunked") != std::string::npos)
    {
        size_t pos = headerEnd + 4; // Inizio del body dopo gli header
        size_t endChunkPos = request.find("0\r\n\r\n", pos);

        if (endChunkPos == std::string::npos)
            return 0; // **Richiesta incompleta, aspettiamo altri dati**

        while (pos < endChunkPos)
        {
            // Trova la fine della dimensione del chunk
            size_t chunkSizeEnd = request.find("\r\n", pos);
            if (chunkSizeEnd == std::string::npos)
                return 0; // **Chunk incompleto, attendiamo altri dati**

            // Converti la dimensione del chunk in intero
            int chunkSize = std::strtol(request.substr(pos, chunkSizeEnd - pos).c_str(), NULL, 16);
            pos = chunkSizeEnd + 2; // Sposta il puntatore dopo il CRLF

            if (chunkSize == 0)
                break; // **Fine del chunked encoding**

            // Assicurati che il chunk sia completo
            if (request.size() < pos + chunkSize + 2)
                return 0; // **Chunk incompleto, aspettiamo altri dati**

            // Aggiungi il chunk al body senza duplicazioni
            body.append(request.substr(pos, chunkSize));
            pos += chunkSize + 2; // Salta al prossimo chunk (+2 per il CRLF)
        }

        return 1; // **Request completa**
    }

    // Se c'è `Content-Length`, controlliamo se il body è completo
    if (headers.find("Content-Length") != headers.end())
    {
        int contentLength = std::atoi(headers["Content-Length"].c_str());
        size_t bodyStart = headerEnd + 4; // Dopo \r\n\r\n
        size_t bodyLength = request.size() - bodyStart;

        if (bodyLength < (size_t)contentLength)
            return 0; // Request ancora incompleta

        body = request.substr(bodyStart, contentLength);
        return 1; // Request completa
    }

    // Se non c'è né `Content-Length` né `Transfer-Encoding`, la richiesta è completa
    return 1;
} */

void HttpRequest::clear()
{
    method.clear();
    path.clear();
    headers.clear();
    body.clear();
}

std::string HttpResponse::toString() const
{
	std::ostringstream requestStream;
	
	requestStream << "HTTP/1.1 " << statusCode << " " << statusMessage << "\r\n";
	requestStream << "Server: webzerv/TeamForzaAgain\r\n";
	requestStream << "Content-Type: text/html\r\n";
	requestStream << "Content-Length: " << body.size() << "\r\n";
	requestStream << "\r\n";
	requestStream << body;

	return requestStream.str();
}
