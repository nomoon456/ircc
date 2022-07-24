/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   MODE.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mravily <mravily@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/07/15 15:47:22 by mravily           #+#    #+#             */
/*   Updated: 2022/07/16 19:00:27 by mravily          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"

/*
** Vérifie si la modestring contient uniquement des modes
** autorisés apr le server
*/
bool checkAllowMode(std::string allowMode, std::string toCheck)
{
	std::string::iterator it(toCheck.begin());
	for (; it != toCheck.end(); it++)
	{
		if ((*it) == '-' || (*it) == '+')
			continue;
		if (allowMode.find((*it)) == std::string::npos)
			return (false);
	}
	return (true);
}

void MODE(irc::Server *srv, irc::User *usr, irc::Command *cmd)
{
	if (!cmd->getParams().size())
		usr->reply(461);
	if (cmd->getParams()[0].find("#") != std::string::npos)
		chanMode(srv, usr, cmd);
	else
		userMode(srv, usr, cmd);
}
