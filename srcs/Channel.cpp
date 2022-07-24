#include "Server.hpp"

bool irc::Channel::isOperator(irc::User *usr)
{
	for (std::vector<irc::User*>::iterator it = _operator.begin(); it != _operator.end(); it++)
		if ((*it) == usr)
			return (true);
	return (false);
}

bool irc::Channel::findMode(std::string modes)
{
	for (std::string::iterator it = modes.begin(); it != modes.end(); it++)
	{
		if (this->_mode.find(*it) == std::string::npos)
			return (false);
	}
	return (true);
}

std::vector<irc::User *> irc::Channel::getUsers()
{
	std::vector<irc::User *> Users(this->getOperator());
	Users.insert(Users.end(), _users.begin(), _users.end());
	return (Users);
}

std::string irc::Channel::getCurrentDate()
{
	time_t now = time(NULL);
	gmtime(&now);
	std::stringstream tt;
	tt << now;
	return (tt.str());
}

std::string irc::Channel::getListUsers()
{
	std::string responseList;

	for (std::vector<irc::User*>::iterator it = _operator.begin(); it != _operator.end(); it++)
		responseList += "@" + (*it)->getNickname() + " ";
	for (std::vector<irc::User*>::iterator it = _users.begin(); it != _users.end(); it++)
		responseList += (*it)->getNickname() + " ";

	return (responseList);
}

std::string irc::Channel::getUserSize()
{
	std::stringstream ss;
	ss << this->_users.size() + this->_operator.size();
	return (ss.str());
}

void irc::Channel::addUser(irc::User *usr) {_users.push_back(usr);};

/*
** @brief Chercher un user dans le list donner et le retire
** @param list liste d'user dans la quelle chercher
** @param toFind User à trouver
** @return Renvoi true si l'user a été trouver
*/
bool findEraseUser(std::vector<irc::User *> &list, irc::User *toFind, irc::Channel *chan, std::string message)
{
	std::vector<irc::User *>::iterator itOpe(list.begin());
	for (; itOpe != list.end(); itOpe++)
	{
		if ((*itOpe) == toFind)
		{
			toFind->broadcast(chan, message, 0);
			list.erase(itOpe);
			return (true);
			if (!list.size())
				break ;
		}
	}
	return (false);
}

void irc::Channel::eraseUser(std::vector<irc::User *>& list, std::string toFind)
{
	std::vector<irc::User *>::iterator it(list.begin());
	for (; it != list.end(); it++)
	{
		std::cout << (*it)->getNickname() << std::endl;
		if (!toFind.compare((*it)->getNickname()))
			break;
	}
	list.erase(it);
}

void irc::Channel::addMode(irc::User* usr, std::string modestring, std::vector<std::string> arg)
{
	std::vector<std::string>::iterator ita(arg.begin());
	std::string::iterator it(modestring.begin());
	std::string::iterator ite(modestring.end());
	ita++;
	std::vector<std::string>::iterator itt(arg.begin());
	for (; itt != arg.end(); itt++)
		std::cout << "ARG: " << (*itt) << std::endl;
	for (; it != ite; it++)
	{
		if ((*it) == 'o' && isOperator(usr) == false)
		{
			usr->reply(481); 
			modestring.erase(modestring.find('o'));
		}
		if ((*it) == 'o' && isOperator(usr) == true)
		{
			irc::User* toModif = findUserChan(getUser(), (*ita++));
			if (!toModif && usr != toModif)
				usr->reply(401);   		// ERR_NOSUCHNICK
			else if (usr != toModif)
			{
				toModif->setMode((*it));
				_operator.push_back(toModif);
				eraseUser(_users, toModif->getNickname());
				usr->addWaitingSend(":" + usr->getClient() + " MODE " + getName() + " +o :" + toModif->getNickname() + CRLF);
				toModif->addWaitingSend(":" + usr->getClient() + " MODE " + getName() + " +o :" + toModif->getNickname() + CRLF);
			}
			modestring.erase(modestring.find('o'));
		}
	}
	if (modestring.size() && isOperator(usr))
		setModes(usr, modestring);
	else
		usr->reply(482, this);
}

void irc::Channel::setModes(irc::User* usr, std::string modestring)
{
	_mode += modestring;
	usr->broadcast(this, " MODE " + getName() + " +" + modestring + " :" + usr->getNickname(), 0);
}

void irc::Channel::rmModes(irc::User* usr, std::string modestring)
{
	std::string::iterator it(modestring.begin());
	for (; it != modestring.end(); it++)
		_mode.erase(_mode.find((*it)));
	usr->broadcast(this, " MODE " + getName() + " -" + modestring + " :" + usr->getNickname(), 0);
}

void irc::Channel::removeMode(irc::User* usr, std::string modestring, std::vector<std::string> arg)
{
	puts("rmv");
	std::vector<std::string>::iterator ita(arg.begin());
	ita++;
	std::string::iterator it(modestring.begin());
	std::string::iterator ite(modestring.end());
	
	std::vector<std::string>::iterator itt(arg.begin());
	for (; itt != arg.end(); itt++)
		std::cout << "ARG: " << (*itt) << std::endl;
	for (; it != ite; it++)
	{
		if ((*it) == 'o' && isOperator(usr) == false)
		{
			usr->reply(481); 
			modestring.erase(modestring.find('o'));
		}
		if ((*it) == 'o' && isOperator(usr) == true)
		{
			irc::User* toModif = findUserChan(getOperator(), (*ita++));
			if (!toModif && usr != toModif)
				usr->reply(401);   		// ERR_NOSUCHNICK
			else if (usr != toModif)
			{
				toModif->getMode().erase(toModif->getMode().find('o'));
				_users.push_back(toModif);
				eraseUser(_operator, toModif->getNickname());
				usr->addWaitingSend(":" + usr->getClient() + " MODE " + getName() + " -o :" + toModif->getNickname() + CRLF);
				toModif->addWaitingSend(":" + usr->getClient() + " MODE " + getName() + " -o :" + toModif->getNickname() + CRLF);
			}
			modestring.erase(modestring.find((*it)));
		}
	}
	if (modestring.size() && isOperator(usr))
		rmModes(usr, modestring);
	else
		usr->reply(482, this);
}

void irc::Channel::removeUser(irc::User *usr, std::string message)
{
	bool find = false;
	if (!(find = findEraseUser(_operator, usr, this, message)))
		find = findEraseUser(_users, usr, this, message);
	if (find == false)
		usr->reply(442, this);
}

void irc::Channel::deleteUser(irc::User *target)
{
	for (std::vector<User *>::iterator it = this->_operator.begin(); it != this->_operator.end(); it++)
	{
		if (*it == target)
		{
			this->_operator.erase(it);
			return;
		}
	}
	for (std::vector<User *>::iterator it = this->_users.begin(); it != this->_users.end(); it++)
	{
		if (*it == target)
		{
			this->_users.erase(it);
			return;
		}
	}
}

void irc::Channel::kickUser(irc::User *usr, irc::User *target, std::string reason)
{
	usr->broadcast(this, " KICK " + this->_name + " " + target->getNickname() + " " + reason, 0);
	deleteUser(target);
}

bool irc::Channel::knowUser(irc::User* usr)
{
	std::vector<User *> users = getUsers();
	for (std::vector<User *>::iterator it = users.begin(); it != users.end(); it++)
	{
		if ((*it) == usr)
			return (true);
	}
	return (false);
}

irc::Channel::Channel(bool type, std::string name, irc::User* ope, std::string pass) : _private(type), _name(name), _mode("nt"), _password(pass), _datatime(getCurrentDate())
{
	this->_operator.push_back(ope);
}

irc::Channel::~Channel() {};
