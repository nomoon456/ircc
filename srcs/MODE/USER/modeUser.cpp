/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   modeUser.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mravily <mravily@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/07/16 19:00:40 by mravily           #+#    #+#             */
/*   Updated: 2022/07/16 20:14:01 by mravily          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"

/*
** @brief Affiche les modes de l'utilisateur courant 
** si il existe et prépare la réponse correspondante
*/
void displayMode(irc::Server* srv, irc::User* usr, irc::Command* cmd)
{
	irc::User* user = nullptr;
	user = findUserSrv(srv, cmd->getParams()[0]);

	if (user != usr)
		usr->reply(502);  		// ERR_USERSDONTMATCH
	else if (!user)
		usr->reply(401);   		// ERR_NOSUCHNICK 
	else
		usr->reply(221);  		// RPL_UMODEIS
}

void changeModeUser(irc::Server* srv, irc::User* usr, irc::Command* cmd)
{
	irc::User* user = nullptr;
	user = findUserSrv(srv, cmd->getParams()[0]);
	if (user != usr)
		usr->reply(502);  		// ERR_USERSDONTMATCH
	else if (!user)
		usr->reply(401);   		// ERR_NOSUCHNICK 
	else if (!checkAllowMode(srv->getUsrMode(), cmd->getParams()[1]))
		user->reply(501);		// ERR_UMODEUNKNOWNFLAG
	else
		user->setModes(cmd->getParams()[1]);
}

void userMode(irc::Server *srv, irc::User *usr, irc::Command *cmd)
{	
	if (!cmd->getParams().size())
		usr->reply(461);
	if (cmd->getParams().size() == 1)
		displayMode(srv, usr, cmd);
	else
		changeModeUser(srv, usr, cmd);
}
