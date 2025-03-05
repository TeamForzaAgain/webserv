#ifndef LOGICVALIDATOR_HPP
# define LOGICVALIDATOR_HPP

# include <iostream>
# include <sstream>
# include <stack>
# include <set>
# include <cstdlib>
# include "Include.hpp"

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
		void validateReturn(const std::string &value, int lineNumber);
		bool isValidUrl(const std::string &url);
		bool isValidHttpStatus(const std::string &code);
	public:
		LogicValidator(std::stringstream &stream);
		~LogicValidator();
		bool validate();
};

#endif
