#include "LogicValidator.hpp"
#include <iostream>
#include <sstream>
#include <string>
#include <vector>


LogicValidator::LogicValidator(std::stringstream &stream) : stream(stream) {}

static std::vector<std::string> split (const std::string &s, char delim)
{
    std::vector<std::string> result;
    std::stringstream ss (s);
    std::string item;

    while (getline (ss, item, delim))
	{
        result.push_back (item);
    }

    return (result);
}

bool LogicValidator::isValidPort(const std::string &value)
{
	std::vector<std::string> splitted = split(value, ':');
	bool isValidIp = false;

	if (splitted.size() > 2)
	{
		std::ostringstream oss;
		oss << RED << "Porta non valida `" << value << "` troppi `:`." << RESET;
		std::cerr << oss.str() << std::endl;
		return false;
	}
	std::string ipPart;
	std::string portPart;

	if (splitted.size() == 2)
	{
		ipPart = splitted[0];
		portPart = splitted[1];
	}
	else
	{
		isValidIp = true;
		portPart = splitted[0];
	}

	if (ipPart == "localhost")
	{
		isValidIp = true;
	}
	if (isValidIp == false)
	{
		std::string ip = ipPart;
		std::vector<std::string> splittedIp = split(ip, '.');
		if (splittedIp.size() != 4)
		{
			std::ostringstream oss;
			oss << RED << "Porta non valida `" << value << "` IP non valido." << RESET;
			std::cerr << oss.str() << std::endl;
			return false;
		}
		for (size_t i = 0; i < splittedIp.size(); i++)
		{
			if (!isdigit(splittedIp[i][0]))
			{
				std::ostringstream oss;
				oss << RED << "Porta non valida `" << value << "` valore non numerico." << RESET;
				std::cerr << oss.str() << std::endl;
				return false;
			}
			int ipPart = std::atoi(splittedIp[i].c_str());
			if (ipPart < 0 || ipPart > 255)
			{
				std::ostringstream oss;
				oss << RED << "Porta non valida `" << ipPart << "` fuori dal range 0-255." << RESET;
				std::cerr << oss.str() << std::endl;
				return false;
			}
		}
		isValidIp = true;
	}

	int	port;

	for (size_t i = 0; i < portPart.size(); i++)
	{
		if (!isdigit(portPart[i]))
		{
			std::ostringstream oss;
			oss << RED << "Porta non valida `" << portPart[i] << "` valore non numerico." << RESET;
			std::cerr << oss.str() << std::endl;
			return false;
		}
	}
	port = std::atoi(portPart.c_str());
	if (port < 0 || port > 65535)
	{
		std::ostringstream oss;
		oss << RED << "Porta non valida `" << port << "` fuori dal range 0-65535." << RESET;
		std::cerr << oss.str() << std::endl;
		return false;
	}
	return (true);
}

bool LogicValidator::isValidSize(const std::string &value)
{
	int	size;

	for (size_t i = 0; i < value.size(); i++)
	{
		if (!isdigit(value[i]))
		{
			std::ostringstream oss;
			oss << RED << "Size non valida `" << value[i] << "` valore non numerico." << RESET;
			std::cerr << oss.str() << std::endl;
			return false;
		}
	}
	size = std::atoi(value.c_str());
	if (size < 0 || size > 100000)
	{
		std::ostringstream oss;
		oss << RED << "size non valida `" << size << "` fuori dal range 0-100000." << RESET;
		std::cerr << oss.str() << std::endl;
		return false;
	}
	return (true);
}

bool LogicValidator::isValidMethod(const std::string &method)
{
	static const std::set<std::string> validMethods;

	if (validMethods.empty())
	{
		const_cast<std::set<std::string>&>(validMethods).insert("GET");
		const_cast<std::set<std::string>&>(validMethods).insert("POST");
		const_cast<std::set<std::string>&>(validMethods).insert("DELETE");
	}
	return (validMethods.find(method) != validMethods.end());
}

bool LogicValidator::validateBlockStructure(const std::string &token, int lineNumber)
{
	if (token == "server")
	{
		if (!blockStack.empty())
		{
			std::ostringstream oss;
			oss << "Riga " << lineNumber << ": `server` non può stare dentro un altro blocco.";
			throw std::invalid_argument(oss.str());
		}
		blockStack.push("server");
	} else if (token == "location" || token == "default_location")
	{
		if (blockStack.empty() || blockStack.top() != "server")
		{
			std::ostringstream oss;
			oss << "Riga " << lineNumber << ": `" << token << "` deve stare dentro un `server`.";
			throw std::invalid_argument(oss.str());
		}
		blockStack.push(token);
	} else if (token == "{")
	{
		return (true);
	} else if (token == "}")
	{
		if (blockStack.empty())
		{
			std::ostringstream oss;
			oss << "Riga " << lineNumber << ": Chiusura `}` senza blocco corrispondente.";
			throw std::invalid_argument(oss.str());
		}
		blockStack.pop();
	}
	return (true);
}

static const std::set<std::string> &getValidBooleans()
{
	static std::set<std::string> validBooleans;

	if (validBooleans.empty())
	{
		validBooleans.insert("on");
		validBooleans.insert("off");
	}
	return (validBooleans);
}

bool LogicValidator::validateDirective(const std::string &token, const std::string &value, int lineNumber)
{
	if (token == "listen")
	{
		if (!isValidPort(value))
		{
			std::ostringstream oss;
			oss << "Riga " << lineNumber << ": Porta non valida `" << value << "`.";
			throw std::invalid_argument(oss.str());
		}
	} else if (token == "methods")
	{
		std::istringstream iss(value);
		std::string method;
		while (iss >> method)
		{
			if (!isValidMethod(method))
			{
				std::ostringstream oss;
				oss << "Riga " << lineNumber << ": Metodo HTTP non valido `" << method << "`.";
				throw std::invalid_argument(oss.str());
			}
		}
	} else if (token == "dir_listing" || token == "alias" || token == "upload" || token == "cgi")
	{
		if (getValidBooleans().find(value) == getValidBooleans().end())
		{
			std::ostringstream oss;
			oss << "Riga " << lineNumber << ": `" << token << "` accetta solo `on` o `off`, trovato `" << value << "`.";
			throw std::invalid_argument(oss.str());
		}
	} else if (token == "max_body_size")
	{
		if (!isValidSize(value))
		{
			std::ostringstream oss;
			oss << "Riga " << lineNumber << ": `max_body_size` non valida `" << value << "`.";
			throw std::invalid_argument(oss.str());
		}
	}
	return (true);
}

bool LogicValidator::validate()
{
	std::string line;
	int lineNumber = 0;

	while (std::getline(stream, line))
	{
		lineNumber++;
		std::istringstream iss(line);
		std::string token, value, fullValue;

		while (iss >> token)
		{
			validateBlockStructure(token, lineNumber);

			// **Se la direttiva è tra quelle che richiedono un valore**
			if (token == "listen" || token == "methods" || token == "dir_listing" ||
				token == "alias" || token == "upload" || token == "cgi" || token == "max_body_size")
			{
				fullValue = "";

				// **Leggiamo i valori multipli**
				while (iss >> value)
				{
					if (value.find(';') != std::string::npos) {
						fullValue += value.substr(0, value.find(';')); // Prendi il valore senza `;`
						break; // Terminiamo la lettura
					} else {
						fullValue += value + " ";
					}
				}

				// **Se non è stato letto alcun valore, errore**
				if (fullValue.empty())
				{
					std::ostringstream oss;
					oss << "Riga " << lineNumber << ": Direttiva `" << token << "` senza valore.";
					throw std::invalid_argument(oss.str());
				}

				// **Validiamo il valore della direttiva**
				validateDirective(token, fullValue, lineNumber);
			}
		}
	}

	if (!blockStack.empty())
	{
		std::ostringstream oss;
		oss << "Blocco `" << blockStack.top() << "` non chiuso correttamente.";
		throw std::invalid_argument(oss.str());
	}

	std::cout << GREEN << "[SUCCESS] Validazione logica completata con successo!" << RESET << std::endl;
	return (true);
}

