#include "SessionManager.hpp"
#include <cstdlib>
#include <sstream>

std::string SessionManager::getSession(const std::string &session_id) const
{
	std::map<std::string, std::string>::const_iterator it = sessions.find(session_id);
	if (it != sessions.end())
		return it->second;
	return "";
}

std::string SessionManager::createSession()
{
	std::stringstream ss;
	ss << rand();
	std::string session_id = ss.str();

	sessions[session_id] = "User Data";
	return session_id;
}
