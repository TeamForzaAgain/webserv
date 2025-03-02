#ifndef SESSIONMANAGER_HPP
#define SESSIONMANAGER_HPP

#include <map>
#include <string>

class SessionManager
{
private:
	std::map<std::string, std::string> sessions;

public:
	std::string getSession(const std::string &session_id) const;
	std::string createSession();
};

#endif
