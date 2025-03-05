#ifndef SYNTAXVALIDATOR_HPP
# define SYNTAXVALIDATOR_HPP

# include <iostream>
# include <sstream>
# include <stack>
# include <set>
# include "Include.hpp"

class SyntaxValidator
{
	private:
		std::stringstream &stream;
		std::stack<char> braceStack;
		std::set<std::string> validTokens;

		bool isValidToken(const std::string &token);
		bool checkSemicolon(const std::string &token, const std::string &line, int lineNumber);
		bool validateBraces(const std::string &token, int lineNumber);

	public:
		SyntaxValidator(std::stringstream &stream);
		~SyntaxValidator();
		bool validate();
};

#endif
