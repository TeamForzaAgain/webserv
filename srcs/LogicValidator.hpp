#ifndef LOGICVALIDATOR_HPP
# define LOGICVALIDATOR_HPP

# include <iostream>
# include <sstream>
# include <stack>
# include <set>
# include <cstdlib>

# define GREEN "\033[32m"
# define RED "\033[31m"
# define RESET "\033[0m"

class LogicValidator
{
	private:
		std::stringstream &stream;
		std::stack<std::string> blockStack;

		bool isValidPort(const std::string &value);
		bool isValidSize(const std::string &value);
		bool isValidMethod(const std::string &method);
		bool validateBlockStructure(const std::string &token, int lineNumber);
		bool validateDirective(const std::string &token, const std::string &value, int lineNumber);

	public:
		LogicValidator(std::stringstream &stream);
		~LogicValidator();
		bool validate();
};

#endif
