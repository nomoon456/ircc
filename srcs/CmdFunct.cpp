#include "Server.hpp"

void PASS(irc::Server *srv, irc::User *usr, irc::Command *cmd)
{
	std::cout << "PASS Funct:" << std::endl;
	std::cout << usr->printStatus() << std::endl;
	std::cout << "Serv.Pass: [" << srv->getPassword() << "]" << std::endl;
	std::cout << "cmd->getParams()[0]: [" << cmd->getParams()[0] << "]" << std::endl;
	if (!cmd->getParams().size())
		usr->reply(461);    	// ERR_NEEDMOREPARAMS
	else if (usr->getStatus() == irc::CONNECTED && !cmd->getParams()[0].compare(srv->getPassword()))
	{
		usr->setStatus(irc::AUTHENTICATED);
		usr->setBits(0);
	}
	else if (usr->getStatus() != irc::CONNECTED && usr->getStatus() != irc::LEAVE)
		usr->reply(462);		// ERR_ALREADYREGISTERED
	else
		usr->reply(464);		// ERR_PASSWDMISMATCH
}

void USER(irc::Server *srv, irc::User *usr, irc::Command *cmd)
{
	usr->setBits(2);
	if (cmd->getParams().size() < 3)
		usr->reply(461);
	else if (usr->getStatus() == irc::REGISTERED)
		usr->reply(462);
	else
	{
		usr->setUsername(cmd->getParams()[0]);
		usr->setRealname(cmd->getTrailer());
	}
	(void)srv;
}

bool chanExist(irc::Server *srv, std::string toFind)
{

	std::vector<irc::Channel *> Chan(srv->getChannels());
	std::vector<irc::Channel *>::iterator it(Chan.begin());
	for (; it != Chan.end(); it++)
	{
		std::cout << (*it)->getName() << std::endl;
		if (!toFind.compare((*it)->getName()))
			return (true);
	}
	return (false);
}

void JOIN(irc::Server *srv, irc::User *usr, irc::Command *cmd)
{
	if (!cmd->getParams().size())
		usr->reply(461);

	std::vector<std::string> chanNames = split(cmd->getParams()[0], ",");
	std::vector<std::string> keys;
	std::vector<std::string>::iterator itPass;
	if (cmd->getParams().size() > 1)
	{
		std::vector<std::string> keys = split(cmd->getParams()[1], ",");
		itPass = keys.begin();
	}
	std::vector<std::string>::iterator itNames(chanNames.begin());
	irc::Channel* chan = nullptr;
	for (; itNames != chanNames.end(); itNames++)
	{
		if (!(chan = findChan(srv, (*itNames))))
			srv->createChan((*itNames), usr);
		else
		{
			puts("JOIN CHANNEL");
			srv->joinChan(chan, usr);
		}
	}
	puts("OUT JOIN");
}

void PING(irc::Server *srv, irc::User *usr, irc::Command *cmd)
{
	(void)srv;
	if (!cmd->getParams().size())
		usr->reply(461);
	usr->addWaitingSend(":" + usr->getNickname() + "!" + usr->getUsername() + "@" + usr->getHostname() + " " + "PONG :" + cmd->getParams()[0] + CRLF);
	usr->setStatus(irc::ONLINE);
}


/* @brief Recherche dans les channels existant et quitte le channel avec une reason
** @param channel list des channels à quitter
** @param reason Raison pour la quel l'user quitte le channel
*/
void PART(irc::Server *srv, irc::User *usr, irc::Command *cmd)
{
	if (!cmd->getParams().size())
		usr->reply(461);

	irc::Channel* chan = nullptr;
	std::vector<std::string> chanNames = split(cmd->getParams()[0], ",");
	std::vector<std::string>::iterator itNames(chanNames.begin());
	for (; itNames != chanNames.end(); itNames++)
	{
		if (!(chan = findChan(srv, (*itNames))))
		{
			irc::Channel* tmp = new irc::Channel(false, (*itNames), usr);
			usr->reply(403, tmp);
			delete tmp;
		}
		else
			chan->removeUser(usr, (" PART :" + chan->getName()));
	}
}

void QUIT(irc::Server *srv, irc::User *usr, irc::Command *cmd)
{
	(void)srv;
	usr->setReason(cmd->getTrailer());
	usr->setStatus(irc::LEAVE);
}

void PRIVMSG(irc::Server *srv, irc::User *usr, irc::Command *cmd)
{
	std::vector<std::string> Targets = split(cmd->getParams()[0], ",");
	std::string msg = cmd->getTrailer();

	if (!msg.size())
	{
		usr->reply(412);
		return;
	}
	irc::Channel* chan;
	irc::User* userTarget;
	// si arg n'est pas channel, go msg prive to user
	for (std::vector<std::string>::iterator it = Targets.begin(); it != Targets.end(); it++)
	{
		if ((chan = findChan(srv, (*it))) != nullptr) // si channel diffusion message dans le channel
		{
			//si usr n'est pas dans le channel et que le chan est mode +n et qu'il n'est pas ops
			if (chan->knowUser(usr) == false && chan->findMode("n") == true && usr->getOperator() == false)
				usr->reply(404, chan);
			else
				usr->broadcast(chan, ":" + usr->getClient() + " " + cmd->getPrefix() + " " + (*it) + " :" + msg + CRLF, usr);
		}
		else // sinon msg prive to user
		{
			userTarget = srv->getUserByNick(*it);
			if (userTarget != nullptr)
			{
				userTarget->addWaitingSend(":" + usr->getClient() + " " + cmd->getPrefix() + " " + (*it) + " :" + msg + CRLF);
				userTarget->processReply();
			}
			else
				usr->reply(401);
		}
	}
}

void LIST(irc::Server *srv, irc::User *usr, irc::Command *cmd)
{
	(void)cmd;
	usr->reply(321);

	std::vector<irc::Channel *> channels; // va devenir la list des channels a afficher

	if (cmd->getParams()[0].size() != 0) // si il y a arguments (LIST <channel>,<channel>...)
	{
		std::vector<std::string> channelNames = split(cmd->getParams()[0], ","); //recup les differents names channel
		channels = srv->getListChannelByName(channelNames);
	}
	else // sinon recup all channels du serveur pour les afficher
		channels = srv->getChannels();

	for (std::vector<irc::Channel *>::iterator it = channels.begin(); it != channels.end(); it++)
	{
		if ((*it)->isPrivate() == false || (*it)->knowUser(usr) == true) // si channel prive (&) affiche seulement si user dedans
			usr->reply(322, (*it));
	}
	usr->reply(323);
}

void TOPIC(irc::Server *srv, irc::User *usr, irc::Command *cmd)
{
	irc::Channel *chan;

	if ((chan = findChan(srv, cmd->getParams()[0])) == NULL)
	{
		usr->reply(461, chan); // no parametres
		return;
	}

	std::string newTopic = cmd->getTrailer();

	if (!newTopic.size())
		usr->reply(331, chan);
	else
	{
		if (chan->knowUser(usr) == false) // si user pas present dans channel
			usr->reply(442, chan);
		else if (chan->findMode("t") == true && chan->isOperator(usr) == false) // si channel mode +t et usr n'est pas operator(A REGLER !!)
			usr->reply(482, chan);
		else
		{
			chan->setTopic(newTopic);
			std::string response = usr->getReplies(332, chan);
			usr->broadcast(chan, response, 0);
		}
	}
}

void KILL(irc::Server *srv, irc::User *usr, irc::Command *cmd)
{
	if (!cmd->getParams().size() || cmd->getParams().size() != 2)
		usr->reply(461); //ERR_NEEDMOREPARAMS
	if (!usr->getMode().find('o'))
		usr->reply(481); //ERR_NOPRIVILEGES
	irc::User* user = NULL;
	if (!(user = findUserSrv(srv, cmd->getParams()[0])))
		usr->reply(401);
	else
	{
		user->setStatus(irc::LEAVE);
		user->setReason(cmd->getParams()[1]);
		srv->broadcast("Killed (" + usr->getNickname() + " " + cmd->getParams()[1] + ")");
		user->addWaitingSend(":" + user->getClient() + " ERROR: Closing link: IRC Hobbs Killed (" + usr->getNickname() + " " + cmd->getParams()[1] + ")" + CRLF);
	}

}

void OPER(irc::Server *srv, irc::User *usr, irc::Command *cmd)
{
	if (cmd->getParams().size() < 2)
		usr->reply(461);
	else if (!srv->getOperName().compare(cmd->getParams()[0]) && !srv->getOperPassword().compare(cmd->getParams()[1]))
	{
		usr->setOper(true);
		usr->reply(381);
	}
	else
		usr->reply(464);
}

void KICK(irc::Server *srv, irc::User *usr, irc::Command *cmd)
{
	if (!cmd->getParams()[0].size() || !cmd->getParams()[1].size())
	{
		usr->reply(461);
		return;
	}
	irc::Channel *chan;
	irc::User *target;
	std::string channelName = cmd->getParams()[0];
	std::vector<std::string> names = split(cmd->getParams()[1], ",");
	std::string reason = cmd->getTrailer();
	if (!reason.size())
		reason = "no reason";
	
	if ((chan = findChan(srv, channelName)) == NULL)
	{
		usr->reply(401, chan);
		return;
	}	
	else if (chan->knowUser(usr) == false)
	{
		usr->reply(442, chan);
		return;
	}

	for (std::vector<std::string>::iterator it = names.begin(); it != names.end(); it++)
	{
		if ((target = srv->getUserByNick(*it)) == NULL)
			usr->reply(403, chan);
		else if (chan->knowUser(target) == false)
			usr->reply(441, chan);
		else
		{
			chan->kickUser(usr, target, reason);
		}
	}
}

void INVITE(irc::Server *srv, irc::User *usr, irc::Command *cmd)
{
	if (!cmd->getParams()[0].size() || !cmd->getParams()[1].size())
	{
		usr->reply(461);
		return;
	}
	std::string nameTarget = cmd->getParams()[0];
	std::string nameChannel = cmd->getParams()[1];
	irc::User *target;
	irc::Channel *chan = findChan(srv, nameChannel);
	if (chan == NULL)
	{
		usr->reply(401); // devrait etre 403 NOSUCHCHANNEL
		return;
	}
	else if (chan->knowUser(usr) == false)
	{
		usr->reply(442);
		return;
	}
	else if ((target = srv->getUserByNick(nameTarget)) == NULL)
	{
		usr->reply(401);
		return;
	}
	else if (chan->knowUser(target) == true)
	{
		usr->reply(443, chan);
		return;
	}
	else if (chan->isOperator(usr) == false)
	{
		usr->reply(482, chan);
		return;
	}
	else
	{
		std::string response = ":" + usr->getClient() + " INVITE " + nameTarget + " " + nameChannel + CRLF;
		send(target->getFd(), response.c_str(), response.length(), 0);
		usr->addWaitingSend(":" + usr->getClient() + " 341 " + usr->getNickname() + " " + nameTarget + " :" + nameChannel + CRLF);
		if (target->haveInvitation(nameChannel) == false) //eviter doublon 
			target->addInvitation(nameChannel);
	}

}

void NAMES(irc::Server *srv, irc::User *usr, irc::Command *cmd)
{
	std::vector<std::string> channelNames = cmd->getParams();

	irc::Channel *chan;
	for (std::vector<std::string>::iterator it = channelNames.begin(); it != channelNames.end(); it++)
	{
		if ((chan = findChan(srv, *it)) != NULL)
		{
			usr->reply(353, chan);
			usr->reply(366, chan);
		}
		else
			usr->reply(401);
	}
}
