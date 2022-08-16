#include "Server.hpp"

void irc::Server::setDatatime()
{
	time_t tmm = time(0);
	this->_datatime = asctime(gmtime(&tmm));
	this->_datatime.erase(--_datatime.end());
};

void irc::Server::setPassword(std::string pass) {this->_password = pass;};
std::string irc::Server::getPassword() {return (_password);};
std::string irc::Server::getVersion() {return (_version);};
std::string irc::Server::getDatatime() {return (_datatime);};
std::map<int, irc::User *> irc::Server::getUsers() {return (_users);};
std::string irc::Server::getUsrMode() {return (_usrMode);};
std::string irc::Server::getChanMode() {return (_chanMode);};

irc::Channel* irc::Server::getChannelByName(std::string name)
{
	for (std::vector<irc::Channel>::iterator it = this->_channels.begin(); it != this->_channels.end(); it++)
	{
		std::string currentName = it->getName();
		if (currentName == name || !currentName.compare(1, name.size(), name))
			return (&(*it));
	}
	return (NULL);
}
// fonction pour renvoyer un vector de channels bases sur une liste de noms
// exemple /LIST <channel1>,<channel2> ..(on doit lister seulement channel1 et channel2)
std::vector<irc::Channel *> irc::Server::getListChannelByName(std::vector<std::string> name)
{
	std::vector<Channel *> list;

	for (std::vector<std::string>::iterator it = name.begin(); it != name.end(); it++)
	{
		Channel* chan = getChannelByName((*it));
		if (chan != NULL)
			list.push_back(chan);
	}
	return (list);
}

irc::User* irc::Server::getUserByNick(std::string nick)
{
	for (std::map<int, irc::User *>::iterator it = _users.begin(); it != _users.end(); it++)
	{
		if (nick.compare(it->second->getNickname()) == 0)
			return (it->second);
	}
	return (NULL);
}

/*
** @brief Créer un point de communication
** @param domain cette constante désigne les protocoles internet IPv4
** @param type le type indique le style de communication désiré entre les deux participants
** @param protocol le protocole est souvent mis à zéro car l'association de la famille de protocole et du type de communication définit explicitement le protocole de transport.
*/
void	irc::Server::setSocketServer(int domain, int type, int protocol)
{
	if ((this->_socketServer = socket(domain, type, protocol)) < 0)
		DisplayError("Socket: ");
}

/*
** @brief Ajoute socket à la list des sockets surveiller par poll
** @param socket socket to add
*/
void	irc::Server::addSocket(int socket)
{
	this->_pollFds.push_back(pollfd());
	this->_pollFds.back().fd = socket;
	this->_pollFds.back().events = POLLIN;
}

/*
** @brief Manipulation du socket afin qu'il soit non bloquant
** @param F_SETFL Positionner les nouveaux attributs d'état pour le fichier à la valeur indiquée par arg.
** @param O_NONBLOCK Cela évite de bloquer "longtemps" l'ouverture du fichier.
*/
void	irc::Server::manipSocket(int fd, int cmd, int arg)
{
	if (fcntl(fd, cmd, arg) < 0)
		DisplayError("Fcntl: ");
}

/*
** @brief Configure les options du socket
** @param fd Socket à modifier les options
** @param level lvl approprier au protocol TCP
** @param optname Indique les règles permettant la validation des adresses fournies dans un appel bind doivent autoriser la réutilisation des adresses locales
** @param optval un paramètre non nul valide une option booléenne, et zéro l'invalide
** @param optlen taille du param optval
*/
void	irc::Server::configSocketServer()
{
	int opt = 1;
	if (setsockopt(this->_pollFds[0].fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
		DisplayError("SetSockOpt: ");
	manipSocket(this->_pollFds[0].fd, F_SETFL, O_NONBLOCK);
}


/*
** @brief Structure contenant les informations relatives au socket
** @param sin_family famille d'adresses
** @param sin_addr.s_addr INADDR_ANY (0.0.0.0) signifie un attachement à n'importe quelle adresse
** @param sin_port indique sur quel port le serveur se met en attente de connexion
*/
void irc::Server::setAddressServer(char *port)
{
	(this->_addrServer.sin_family) = AF_INET;
	(this->_addrServer.sin_addr.s_addr) = htonl(INADDR_ANY);
	(this->_addrServer.sin_port) = htons(atoi(port));
}

/*
** @brief Affecte l'adresse spécifiée dans addr à la socket référencée par le descripteur de fichier sockfd.
** @param sockfd descripteur de fichier du socket
** @param addr structure d'adresse
** @param addrlen indique la taille, en octets de la struct d'address
*/
void irc::Server::bindAddress()
{
	if (bind(this->_pollFds[0].fd, (const struct sockaddr *)&this->_addrServer, sizeof(this->_addrServer)) < 0)
		DisplayError("Bind: ");
}

/*
** @brief Ecoute des connexions sur une socketServer
** @param sockfd est un descripteur de fichier qui fait référence à une socket
** @param backlog définit une longueur maximale jusqu'à laquelle la file des connexions en attente pour sockfd peut croître
*/
void irc::Server::listenAddress()
{
	if (listen(this->_pollFds[0].fd, (this->_addrServer.sin_port)) < 0)
		DisplayError("Bind: ");
}

/*
** @brief Attends un évenement sur le socketServer
** @param fd fd sur lequel on attend un évenement
** @param nfds Nombres de FD
** @param delai L'argument délai définit une limite supérieure, en millisecondes, sur le temps pendant lequel poll() bloquera.
*/
void irc::Server::monitoring()
{
	if (poll(&(this->_pollFds[0]), (this->_pollFds.size()), (60 * 1000) / 10) < 0)
		DisplayError("Poll: ");
}

/*
** Si le socketServer est en attente de donnée de lecture
** Ajout du client a la list des socket surveiler par poll
*/
void irc::Server::acceptClient()
{
	struct sockaddr_in addrClient;
	socklen_t csize = sizeof(addrClient);
	int socketClient;
	if ((socketClient = accept(this->_pollFds[0].fd, (struct sockaddr *)&addrClient, &csize)) < 0)
		DisplayError("Accept: ");

	this->_users[socketClient] = new User(this, socketClient, addrClient); //want to free
	//----met le premier user du serveur OPERATOR------
	if (this->_users.size() == 1)
		this->_users[socketClient]->setOper(true);
	//------------------------------------------
	std::cout << "[SERVER] Nouvelle connexion client sur le server\n"
	<< "[SERVER] Socket [" << socketClient << "] | IP [" <<  _users[socketClient]->getHostaddr().c_str() << "]\n"
	<< "[SERVER] Authentification en cours..." << std::endl;
	_users[socketClient]->setStatus(CONNECTED);
	addSocket(socketClient);
}

void irc::Server::runtime()
{
	monitoring();
	//check time out user
	if (this->_pollFds[0].revents == POLLIN)
		acceptClient();
	else
	{
		for (std::vector<pollfd>::iterator it = _pollFds.begin(); it != _pollFds.end(); ++it)
			if ((*it).revents == POLLIN)
				this->_users[(*it).fd]->getMessages();
	}

	std::map<int, irc::User *>::iterator it(_users.begin());
	while( it != _users.end())
	{
		if ((*it).second->getStatus() == LEAVE)
			this->deleteUser((*(it++)).second->getFd());
		else {
			(*(it++)).second->processReply();
		}
		if (!_users.size())
			break ;
	}
}

std::vector<irc::Channel *> irc::Server::getChannels()
{
    std::vector<Channel *> vec;

    for (std::vector<Channel>::iterator it = this->_channels.begin(); it != this->_channels.end(); it++)
        vec.push_back(&(*it));

    return (vec);
}

bool getType(std::string name) {return (name[0] == '&');};

void irc::Server::createChan(std::string name, irc::User* usr)
{
	_channels.push_back(Channel(getType(name), name, usr));

	usr->addWaitingSend(":" + usr->getClient() + " JOIN :" + _channels.back().getName() + CRLF);
	usr->reply(353, &_channels.back());
	usr->reply(366, &_channels.back());

	std::cout << "[SERVER] " + usr->getNickname() + " à créer le channel " + _channels.back().getName() << std::endl;
}

void irc::Server::joinChan(irc::Channel* chan, irc::User* usr, std::string password)
{
	(void)password;
	if (chan->findMode("i") == true) // si channel sur invitation
	{
		if (usr->getOperator() == false)
		{
			if (usr->haveInvitation(chan->getName()) == false)
			{
				usr->reply(473, chan);
				return;
			}
			else
				usr->delInvitation(chan->getName());
		}
	}
	chan->addUser(usr);

	usr->broadcast(chan, (" JOIN :" + chan->getName()), 0);
	usr->reply(331, chan);
	usr->addWaitingSend(":" + usr->getClient() + " MODE :" + chan->getName() + " +" + _channels.back().getModes() + CRLF);
	usr->reply(353, chan);
	usr->reply(366, chan);
}

irc::Server::Server(char *port, char *pass) : _version("1.42"), _password(pass), _usrMode("iswo"), _chanMode("opsitnmlbvk"), _oper_name("operator"), _oper_password("password")
{
   	setDatatime();
	setSocketServer(AF_INET, SOCK_STREAM, 0);
	addSocket(this->_socketServer);
	configSocketServer();
	setAddressServer(port);
	bindAddress();
	listenAddress();
}

irc::Server::~Server()
{
    std::vector<pollfd>::iterator it(_pollFds.begin());
    std::map<int, irc::User *>::iterator itu(_users.begin());
    std::vector<irc::Channel>::iterator itc(_channels.begin());

    while (itu != _users.end())
        deleteUser((itu++)->second->getFd());
    while (itc != _channels.end())
        _channels.erase(itc++);
    while (_pollFds.size())
    {
        close((*it).fd);
        _pollFds.erase(it++);
    }
    close(this->_socketServer);
}

void irc::Server::broadcast(std::string message)
{
	std::map<int, User *> users(this->getUsers());
	std::map<int, User *>::iterator itUsers(users.begin());
	for (; itUsers != users.end(); itUsers++)
	{
		(*itUsers).second->addWaitingSend(":" + (*itUsers).second->getClient() + message + CRLF);
		(*itUsers).second->processReply();
	}
}

void irc::Server::deleteUser(int fd)
{
	std::vector<pollfd>::iterator it = _pollFds.begin();
	while (it != _pollFds.end() && (*it).fd != fd)
		it++;
	_pollFds.erase(it);

	std::vector<Channel>::iterator chit = _channels.begin();
	while (chit != _channels.end())
		(*chit++).removeUser(_users[fd], (" QUIT :" + _users[fd]->getReason()));
	_users[fd]->addWaitingSend(":" + _users[fd]->getClient() + " QUIT :" + _users[fd]->getReason() + CRLF);
	_users[fd]->processReply();
	delete _users[fd];
	_users.erase(fd);
	close(fd);
}

std::string irc::Server::getOperName() const { return this->_oper_name; }
std::string irc::Server::getOperPassword() const { return this->_oper_password; }
