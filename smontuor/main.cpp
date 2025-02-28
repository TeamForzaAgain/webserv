#include "LogicValidator.hpp"
#include "Webserv.hpp"
#include <exception>
#include <stdexcept>
#include <sstream>

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

int main(int ac, char **av)
{
	std::stringstream fileStream;
try {
	preliminaryChecks(ac, av, fileStream);


	SyntaxValidator validator(fileStream);
	validator.validate();
	fileStream.clear();
	fileStream.seekg(0, std::ios::beg);
	LogicValidator logicValidator(fileStream);
	logicValidator.validate();

	std::cout << GREEN << "[SUCCESS] File di configurazione validato con successo!" << RESET << std::endl;
}
catch (std::exception &e)
{
	std::cerr << RED << "[ERROR] " << e.what() << RESET << std::endl;
}
	return (0);
}