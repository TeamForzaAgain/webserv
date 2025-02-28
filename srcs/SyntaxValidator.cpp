#include "SyntaxValidator.hpp"

SyntaxValidator::SyntaxValidator(std::stringstream &stream) : stream(stream)
{
	validTokens.insert("server");
	validTokens.insert("location");
	validTokens.insert("default_location");
	validTokens.insert("listen");
	validTokens.insert("server_name");
	validTokens.insert("root");
	validTokens.insert("index");
	validTokens.insert("error_page");
	validTokens.insert("auto_index");
	validTokens.insert("alias");
	validTokens.insert("upload");
	validTokens.insert("cgi");
	validTokens.insert("methods");
	validTokens.insert("max_body_size");
}

bool SyntaxValidator::isValidToken(const std::string &token)
{
	return (validTokens.find(token) != validTokens.end());
}

static const std::set<std::string> &getDirectivesRequiringSemicolon()
{
	static std::set<std::string> directives;

	if (directives.empty())
	{
		directives.insert("listen");
		directives.insert("server_name");
		directives.insert("root");
		directives.insert("index");
		directives.insert("error_page");
		directives.insert("auto_index");
		directives.insert("alias");
		directives.insert("upload");
		directives.insert("cgi");
		directives.insert("methods");
		directives.insert("max_body_size");
	}
	return (directives);
}

bool SyntaxValidator::checkSemicolon(const std::string &token, const std::string &line, int lineNumber)
{
    if (getDirectivesRequiringSemicolon().find(token) != getDirectivesRequiringSemicolon().end())
    {
        size_t pos = line.find(token); // Troviamo la direttiva nella riga
        if (pos != std::string::npos)
        {
            size_t semicolonPos = line.find(';', pos);
            if (semicolonPos == std::string::npos) // Se non trova `;`, errore
            {
				std::ostringstream oss;
				oss << "Riga " << lineNumber << ": Direttiva `" << token << "` senza `;`.";
                throw std::invalid_argument(oss.str());
            }
        }
    }
    return true;
}


bool SyntaxValidator::validateBraces(const std::string &token, int lineNumber)
{
	if (token == "{")
	{
		braceStack.push('{');
	} else if (token == "}")
	{
		if (braceStack.empty())
		{
			std::ostringstream oss;
			oss << "Riga " << lineNumber << ": chiusura `}` senza apertura corrispondente.";
			throw std::invalid_argument(oss.str());
		}
		braceStack.pop();
	}
	return (true);
}

bool SyntaxValidator::validate()
{
    std::string line;
    int lineNumber = 0;
    bool expectBlockStart = false;
    bool expectPath = false;

    while (std::getline(stream, line))
    {
        lineNumber++;

        size_t commentPos = line.find('#');
        if (commentPos != std::string::npos) {
            line = line.substr(0, commentPos);
        }

        std::istringstream iss(line);
        std::string token, directive, value, fullValue;

        while (iss >> token)
        {
            if (expectBlockStart && token != "{")
            {
				std::ostringstream oss;
				oss << "Riga " << lineNumber << ": Atteso `{` dopo `" << directive << "`.";
				throw std::invalid_argument(oss.str());
            }
            expectBlockStart = false;

            if (expectPath)
            {
                expectPath = false;
                expectBlockStart = true;
                continue;
            }

            if (token == "{")
            {
                validateBraces("{", lineNumber);
                continue;
            }

            if (token == "}")
            {
                validateBraces("}", lineNumber);
                continue;
            }

            if (token == "server" || token == "default_location")
            {
                directive = token;
                expectBlockStart = true;
                continue;
            }

            if (token == "location")
            {
                directive = token;
                expectPath = true;
                continue;
            }

            if (isValidToken(token))
            {
                directive = token;
                fullValue = "";

                while (iss >> value)
                {
                    if (value.find(';') != std::string::npos)
                    {
                        fullValue += value.substr(0, value.find(';'));
                        break;
                    }
                    else
                    {
                        fullValue += value + " ";
                    }
                }

                if (fullValue.empty())
                {
					std::ostringstream oss;
					oss << "Riga " << lineNumber << ": Direttiva `" << directive << "` senza valore.";
					throw std::invalid_argument(oss.str());
                }

                if (!checkSemicolon(directive, line, lineNumber))
                {
                    throw std::exception();
                }

                if (directive == "error_page")
                {
                    std::istringstream valueStream(fullValue);
                    std::string code, path;
                    if (!(valueStream >> code >> path))
                    {
						std::ostringstream oss;
						oss << "Riga " << lineNumber << ": `error_page` deve avere un codice e un percorso.";
						throw std::invalid_argument(oss.str());
                    }
                }

                continue;
            }
			std::ostringstream oss;
            oss << "Riga " << lineNumber << ": Token sconosciuto `" << token << "`.";
            throw std::invalid_argument(oss.str());
        }

        if (expectBlockStart)
        {
			std::ostringstream oss;
			oss << "Riga " << lineNumber << ": Atteso `{` dopo `" << directive << "`.";
			throw std::invalid_argument(oss.str());
        }

        if (expectPath)
        {
			std::ostringstream oss;
			oss << "Riga " << lineNumber << ": Atteso percorso dopo `location`.";
			throw std::invalid_argument(oss.str());
        }
    }

    if (!braceStack.empty())
    {
		std::ostringstream oss;
		oss << "Manca una chiusura `}` nel file.";
		throw std::invalid_argument(oss.str());
    }

    std::cout << GREEN << "[SUCCESS] Validazione sintattica completata con successo!" << RESET << std::endl;
    return true;
}
