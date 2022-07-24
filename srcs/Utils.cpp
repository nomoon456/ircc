#include "Server.hpp"

bool is_digits(const std::string &str)
{
    return str.find_first_not_of("0123456789") == std::string::npos;
}

/*
** @brief Recherche sur le server donner un user
** @param server sur le quel rechercher l'user
** @param toFind User portant le nickname contenu dans toFind
** @return Renvoi l'user si il est trouver, sinon nullptr
*/
irc::User* findUserSrv(irc::Server *srv, std::string toFind)
{
	// std::cout << "FindUser->toFind: " << toFind << std::endl;
	std::map<int, irc::User *> Users(srv->getUsers());
	std::map<int, irc::User *>::iterator it(Users.begin());
	for (; it != Users.end(); it++)
	{
		std::cout << (*it).second->getNickname() << std::endl;
		if (!toFind.compare((*it).second->getNickname()))
			return ((*it).second);
	}
	return (NULL);
}

/*
** @brief Recherche sur le channel donner un user
** @param channel sur le quel rechercher l'user
** @param toFind User portant le nickname contenu dans toFind
** @return Renvoi l'user si il est trouver, sinon nullptr
*/
irc::User* findUserChan(std::vector<irc::User *> list, std::string toFind)
{
	// std::cout << "FindUser->toFind: " << toFind << std::endl;
	std::vector<irc::User *>::iterator it(list.begin());
	for (; it != list.end(); it++)
	{
		std::cout << (*it)->getNickname() << std::endl;
		if (!toFind.compare((*it)->getNickname()))
			return ((*it));
	}
	return (NULL);
}
irc::Channel* findChan(irc::Server *srv, std::string toFind)
{
	std::cout << "FindUser->toFind: " << toFind << std::endl;
	std::vector<irc::Channel *> Chan(srv->getChannels());
	std::vector<irc::Channel *>::iterator it(Chan.begin());
	for (; it != Chan.end(); it++)
	{
		std::cout << (*it)->getName() << std::endl;
		if (!toFind.compare((*it)->getName()))
			return (*it);
	}
	return (nullptr);
}