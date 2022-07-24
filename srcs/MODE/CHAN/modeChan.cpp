/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   modeChan.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mravily <mravily@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/07/16 18:59:29 by mravily           #+#    #+#             */
/*   Updated: 2022/07/16 21:25:21 by mravily          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"

/*
** @brief Parcours les params de la commande,
** sépare en deux string les modes à retirer
** et à ajouter avec ses éventuels arguments
** @param channel le channel à modifier
** @param commande la commande avec les modestrings et arguments
*/
void changeModeChan(irc::User* usr, irc::Channel* chan, irc::Command* cmd) 
{
	bool minus = false;
	std::string add;
	std::string rmv;
	std::vector<std::string> arg;
	std::vector<std::string> params(cmd->getParams());
	std::vector<std::string>::iterator it(params.begin());
	std::vector<std::string>::iterator ite(params.end());
	for (; it != ite; it++)
	{
		if ((*it).find('+') != std::string::npos || (*it).find('-') != std::string::npos)
		{
			std::string::iterator its((*it).begin());
			for (; its != (*it).end(); its++)
			{
				if ((*its) == '-')
					minus = true;
				else if ((*its) == '+')
					minus = false;
				else if (minus == false)
					add += (*its);
				else if (minus == true)
					rmv += (*its);
			}
		}
		else
			arg.push_back((*it));
	}
	if (add.size())
		chan->addMode(usr, add, arg);
	if (rmv.size())
		chan->removeMode(usr, rmv, arg);
}


void chanMode(irc::Server *srv, irc::User *usr, irc::Command *cmd)
{
	irc::Channel* chan = nullptr;
	chan = findChan(srv, cmd->getParams()[0]);
	if (!chan)
		usr->reply(403);
	else if (cmd->getParams().size() == 1)
	{
		usr->reply(324, chan);
		usr->reply(329, chan);
	}
	else
	{		
		if (!checkAllowMode(srv->getChanMode(), cmd->getParams()[1]))
			usr->reply(501);		// ERR_UMODEUNKNOWNFLAG
		else
			changeModeChan(usr, chan, cmd);
	}
}
