#include "Server.hpp"

bool checkNickname(irc::Server *srv, std::string nickname)
{
	std::map<int, irc::User *> Users(srv->getUsers());
	std::map<int, irc::User *>::iterator it(Users.begin());
	for (; it != Users.end(); it++)
	{
		if (!nickname.compare((*it).second->getNickname()))
			return (false);
	}
	return (true);
}

bool isLetter(char c) { return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z'); }
bool isSpecial(char c) { return (c >= '[' && c <= '`') || (c >= '{' && c <= '}'); }
bool isDigit(char c) { return (c >= '0' && c <= '9'); }

bool checkChar(std::string nickname)
{
	if (nickname.length() > 9)
		return (false);
	std::string::iterator it(nickname.begin());
	std::string::iterator ite(nickname.end());
	for (; it != ite; ++it)
		if (!isalnum((*it)) && (*it) != '_')
			return (false);
	return (true);
}

void NICK(irc::Server *srv, irc::User *usr, irc::Command *cmd)
{
	if (!cmd->getParams().size())
	{
		usr->reply(431);  // ERR_NONICKNAMEGIVEN
		usr->setStatus(irc::ERROR);
		return ;
	}
	else if (!checkChar(cmd->getParams()[0]))
	{
		usr->reply(432);  // ERR_ERRONEUSNICKNAME
		usr->setStatus(irc::ERROR);
	}
	else if (!checkNickname(srv, cmd->getParams()[0]))
	{
		usr->setNickname(cmd->getParams()[0]);
		usr->reply(433);  // ERR_NICKNAMEINUSE
		usr->setStatus(irc::ERROR);
		return ;
	}
	else if (usr->getStatus() == irc::REGISTERED || usr->getStatus() == irc::ONLINE)
		usr->addWaitingSend(":" + usr->getClient() + " " + "NICK :" + cmd->getParams()[0] + CRLF);
	usr->setNickname(cmd->getParams()[0]);
	usr->setBits(1);
	if (usr->checkBit(0))
		usr->setStatus(irc::AUTHENTICATED);
}