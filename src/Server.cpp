/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rgramati <rgramati@42angouleme.fr>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/08 16:17:28 by rgramati          #+#    #+#             */
/*   Updated: 2025/02/13 18:05:25 by rgramati         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <Server.h>
#include <Client.h>

#include <errno.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>

Server::Server(int port, str password): _flag(IRC_STATUS_OK), _port(port), _password(password) 
{
	IRC_LOG("Server constructor called.");

	_sockfd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
	if (_sockfd == -1)
		throw ServerSocketFailedException();
	
	IRC_OK("socket opened on fd "BOLD(COLOR(GRAY,"[%d]")), _sockfd);
}

Server::~Server(void)
{
	IRC_LOG("Server destructor called.");

	close(_sockfd);

	IRC_OK("closed socket on fd "BOLD(COLOR(GRAY,"[%d]")), _sockfd);
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
	for (size_t id = 0; id < _pollSet.size(); ++id)
	{
		if (_pollSet[id].fd == -1)
		{
			_pollSet.erase(_pollSet.begin() + id);
			continue ;
		}
	}
	return (!err);
}

#define	IRC_ERRNO_NOT_FATAL(e)\
(\
		e == ENETDOWN || \
		e == EPROTO || \
		e == ENOPROTOOPT || \
		e == EHOSTDOWN || \
		e == ENONET || \
		e == EHOSTUNREACH || \
		e == EOPNOTSUPP || \
		e == ENETUNREACH\
)

struct pollfd	*Server::_acceptClient()
{
	IRC_LOG("new client connection.");

	struct sockaddr_in	client;
	socklen_t			len = sizeof(client);

	int fd = accept(_sockfd, (struct sockaddr *)&client, &len);
	if (fd == -1)
	{
		std::cout << errno << std::endl;
		if (IRC_ERRNO_NOT_FATAL(errno))
			IRC_LOG("accept failed. retrying...");
		else
			throw ServerAcceptFailedException();
		return (NULL);
	}

	struct pollfd	client_pfd;
	
	client_pfd.fd = fd;
	client_pfd.events = POLLIN | POLLOUT;
	client_pfd.revents = 0;

	_pollSet.push_back(client_pfd);

	IRC_OK("client accepted on fd "BOLD(COLOR(GRAY,"[%d]"))".", fd);
	return (&_pollSet[_pollSet.size() - 1]);
}

void	Server::_executeCommand(Client *client, const str &command)
{
	UNUSED(client);
	UNUSED(command);
	IRC_LOG(BOLD(COLOR(MAGENTA,"command execution : <%s>")), command.c_str());
}

void	Server::_handleMessage(Client *client)
{
	str	buffer = client->get_buffer();

	while (true)
	{
		size_t	linefeed = buffer.find("\n");
		if (linefeed == std::string::npos)
			break ;

		const str		command = buffer.substr(0, linefeed - 1);
		_executeCommand(client, command);	

		if (command.length() + 2 == buffer.length())
		{
			buffer = "";
			break ;
		}
		buffer = buffer.substr(linefeed + 1, buffer.length());
	}
	client->updateBuffer(buffer);
}

void	Server::start()
{
	IRC_LOG("server started, polling...");

	struct pollfd	server_pfd;

	server_pfd.fd = _sockfd;
	server_pfd.events = POLLIN | POLLOUT;
	_pollSet.push_back(server_pfd);

	std::map<int, Client>	clients;
	Client					*client;

	while (IRC_FLAG_GET(_flag, IRC_STATUS_OK))
	{
		if (_updatePollSet())
			continue ;

		for (size_t id = 0; id < _pollSet.size(); ++id)
		{
			struct pollfd	curr = _pollSet[id];

			if (curr.revents & POLLIN)
			{
				if (id == 0)
				{
					struct pollfd	*client_pfd = _acceptClient();

					clients[client_pfd->fd] = Client(client_pfd);
				}
				else
				{
					client = &clients[curr.fd];

					client->readBytes();
					if (IRC_FLAG_GET(client->get_flag(), IRC_CLIENT_EOT))
					{
						IRC_ERR("GOT MESSAGE <%s>", client->get_buffer().c_str());
						_handleMessage(client);
					}
				}
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
