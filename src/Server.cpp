/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rgramati <rgramati@42angouleme.fr>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/08 16:17:28 by rgramati          #+#    #+#             */
/*   Updated: 2025/02/09 21:16:06 by rgramati         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <Server.h>

#include <errno.h>
#include <arpa/inet.h>
#include <unistd.h>

Server::Server(int port, str password): _flag(IRC_STATUS_OK), _port(port), _password(password) 
{
	_sockfd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
	if (_sockfd == -1)
		throw ServerSocketFailedException();

	IRC_OK("Server constructor called.");
}

Server::~Server(void)
{
	IRC_LOG("closing socket %d", _sockfd);

	close(_sockfd);

	IRC_OK("Server destructor called.");
}

void	Server::_bindSocket() const
{	
	IRC_LOG("binding...");

	sockaddr_in	addr;
	
	addr.sin_family = AF_INET;
	addr.sin_port = htons(_port);
	addr.sin_addr.s_addr = INADDR_ANY;
	
	int err = bind(_sockfd, (struct sockaddr *)&addr, sizeof(addr));
	if (err)
		throw ServerBindFailedException();

	IRC_OK("socket binded to port %d.", _port);
}

#define IRC_CLIENT_CAP	10

void	Server::_listenSocket() const
{
	IRC_LOG("listening...");

	int err = listen(_sockfd, IRC_CLIENT_CAP);
	if (err)
		throw ServerListenFailedException();

	IRC_OK("listening on port %d.", _port);
}

void	Server::initSocket()
{
	_bindSocket();
	_listenSocket();
	IRC_FLAG_SET(_flag, IRC_STATUS_OK);
}

#include <poll.h>

bool	Server::_updatePollSet()
{
	int err = poll(&_pollSet[0], _pollSet.size(), 1);
	if (err < 0)
	{
		if (errno == EAGAIN)
			IRC_ERR("poll failed "COLOR(RED,"[EAGAIN]")", trying again...");
		if (errno == EAGAIN || errno == EINTR)
			return (true);
		throw ServerPollFailedException();
	}
	return (!err);
}

void	Server::start()
{
	IRC_LOG("server started, polling...");

	struct pollfd	pfd;

	pfd.fd = _sockfd;
	pfd.events = POLLIN | POLLOUT;
	_pollSet.push_back(pfd);

	while (IRC_FLAG_GET(_flag, IRC_STATUS_OK))
	{
		if (_updatePollSet())
			continue ;
		
		for (size_t id = 0; id < _pollSet.size(); ++id)
		{
			if (_pollSet[id].fd == -1)
			{
				_pollSet.erase(_pollSet.begin() + id);
				continue ;
			}
		}
	}

	IRC_OK("server has been stopped.");
}

void	Server::serverInfo() const
{	
	IRC_LOG
	(
		"Server:\n"
		"\tsocket on file descriptor "BOLD(COLOR(GRAY,"[%d]"))"\n"
		"\tlistening on port         "BOLD(COLOR(GRAY,"[%d]"))"\n",
		_sockfd,
		_port
	);
}
