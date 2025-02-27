/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rgramati <rgramati@42angouleme.fr>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/25 19:32:37 by rgramati          #+#    #+#             */
/*   Updated: 2025/02/27 01:21:13 by yroussea         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "IRCArchitect.h"
extern volatile bool	interrupt;

#include <errno.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <ctime>

#include <Server.h>
#include "commands/pass.h"
#include "commands/nick.h"
#include "commands/user.h"
#include "commands/quit.h"
#include "commands/pong.h"

#include "commands/join.h"
#include "commands/invite.h"
#include "commands/kick.h"
#include "commands/mode.h"
#include "commands/topic.h"
#include "commands/part.h"
#include "commands/privmsg.h"

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

void	Server::_listenSocket() const
{
	IRC_LOG("listening...");

	int err = listen(_sockfd, 10);
	if (err)
		throw ServerListenFailedException();

	IRC_OK("listening on port %d.", _port);
}

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
			return (true);
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

void	Server::_connectClient(int fd)
{
	struct pollfd	client_pfd;
	
	client_pfd.fd = fd;
	client_pfd.events = POLLIN | POLLOUT;
	client_pfd.revents = 0;

	_pollSet.push_back(client_pfd);
}

void	Server::_disconnectClient(Client &client)
{
	IRC_LOG("disconnecting client, fd = " BOLD(COLOR(GRAY,"[%d]")), client.get_fd());

	client.disconnect();
	_clients.erase(client.get_fd());
}

int32_t	Server::_acceptClient(void)
{
	IRC_LOG("new client connection.");

	struct sockaddr_in	client;
	socklen_t			len = sizeof(client);

	int fd = accept(_sockfd, (struct sockaddr *)&client, &len);
	if (fd == -1)
	{
		if (IRC_ERRNO_NOT_FATAL(errno))
			IRC_LOG("accept failed. retrying...");
		else
			throw ServerAcceptFailedException();
		return (-1);
	}
	IRC_OK("client accepted, fd = "BOLD(COLOR(GRAY,"[%d]"))".", fd);

	if (_clients.size() == IRC_CLIENT_CAP)
	{
		IRC_WARN("too many clients, connection refused.");
		close(fd);
		return (-1);
	}
	_connectClient(fd);
	IRC_OK("client connected, fd = "BOLD(COLOR(GRAY,"[%d]"))".", fd);

	return (fd);
}

#include <ctime>
void	Server::_welcomeClient(Client *client)
{
	const char	*username = client->get_username().c_str();
	const str	created = str(RPL_MSG_CREATED) + str(ctime(&_startTime));

	_send(client, _architect.RPL_WELCOME (username, (RPL_MSG_WELCOME + client->get_username()).c_str()));
	_send(client, _architect.RPL_YOURHOST(username));
	_send(client, _architect.RPL_CREATED(username, created.substr(0, created.length() - 1).c_str()));
	_send(client, _architect.RPL_MYINFO(username, "ft_irc", "0.0", "o", "ikl"));
	_send(client, _architect.RPL_ISUPPORT(username, "NICKLEN=9"));
	_send(client, _architect.RPL_MOTDSTART(username));
	_send(client, _architect.RPL_MOTD(username));
	_send(client, _architect.RPL_ENDOFMOTD(username));
}

void	Server::_registerClient(Client *client)
{
	if (client->get_lastPass() != _password)
		goto	passwdMismatch;

	_send(client, "PING ft_irc");
	client->set_flag(client->get_flag() | IRC_CLIENT_PINGED);
	return ;

passwdMismatch:
	_send(client, _architect.ERR_PASSWDMISMATCH(client->get_nickname().c_str()));
	_send(client, "ERROR");
	client->disconnect();
}

void	Server::_executeCommand(Client *client, const str &command)
{
	_seeker.feedString(command);
	_seeker.rebuild(R_COMMAND_MNEMO);
	if (!_seeker.consume())
	{
		IRC_WARN(BOLD(COLOR(YELLOW,"INVALID MNEMONIC.")));
		return ;
	}

	std::vector<str>	&argv = _seeker.get_matches();
	const str			strip = _seeker.get_string();	
	const str			mnemo = argv[0];

	_seeker.feedString(argv[0]);
	_seeker.rebuild(R_IRC_VALID_COMMANDS);
	if (!_seeker.match())
	{
		IRC_WARN(BOLD(COLOR(YELLOW,"COMMAND NOT SUPPORTED.")));
		return ;
	}

	if (!(client->get_flag() & IRC_CLIENT_AUTH))
	{
		_seeker.rebuild(R_IRC_NOAUTH_COMMANDS);
		if (!_seeker.consume())
		{
			IRC_WARN(BOLD(COLOR(YELLOW,"CLIENT NOT AUTHENTIFIED, IGNORING...")));
			return ;
		}
	}

	IRC_OK("COMMAND " BOLD(COLOR(GREEN,"[%s")) BOLD(COLOR(CYAN,"%s]")) " FUNCTION CALLED !", mnemo.c_str(), strip.c_str());
	(this->*_commandFuncs[mnemo])(client, strip);
}

void	Server::_handleMessage(Client *client)
{
	str					buffer = client->get_buffer();
	std::vector<str>	commands;

	while (true)
	{
		const size_t	linefeed = buffer.find("\n");
		if (!linefeed || linefeed == str::npos)
			break ;
		const size_t	crlf = (buffer.at(linefeed - 1) == '\r');
		const str		command = buffer.substr(0, linefeed - crlf);
	
		commands.push_back(command);
		buffer = buffer.substr(linefeed + 1, buffer.length());
	}
	client->resetBuffer();

	for (IRC_AUTO it = commands.begin(); it != commands.end(); ++it)
	{
		IRC_OK(BOLD(COLOR(GREEN, "EXECUTION CALL [ %s ]")), (*it).c_str());
		_executeCommand(client, *it);
	}
}

void	Server::start()
{
	IRC_LOG("server started, polling...");

	struct pollfd	server_pfd;

	server_pfd.fd = _sockfd;
	server_pfd.events = POLLIN;
	_pollSet.push_back(server_pfd);

	Client					*client;

	while (IRC_FLAG_GET(_flag, IRC_STATUS_OK))
	{
		if (interrupt)
			break ;

		if (_updatePollSet())
			continue ;

		for (size_t id = 0; id < _pollSet.size(); ++id)
		{
			struct pollfd	curr = _pollSet[id];
			if (curr.fd == 0) { continue; }

			if (curr.revents & POLLIN)
			{
				if (id == 0)
				{
					int32_t	client_fd = _acceptClient();
					if (client_fd == -1) { continue ; }

					_clients[client_fd] = Client(0, client_fd);
				}
				else
				{
					client = &_clients[curr.fd];

					client->readBytes();
					if (IRC_FLAG_GET(client->get_flag(), IRC_CLIENT_EOT))
						_handleMessage(client);
					if (IRC_FLAG_GET(client->get_flag(), IRC_CLIENT_EOF))
						_disconnectClient(_clients[curr.fd]);
					IRC_LOG("------------------------------------");
				}
			}
			curr.revents = 0;
		}
	}

	IRC_OK("server has been stopped.");
}
