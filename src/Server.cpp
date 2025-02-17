/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rgramati <rgramati@42angouleme.fr>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/08 16:17:28 by rgramati          #+#    #+#             */
/*   Updated: 2025/02/17 13:35:58 by yroussea         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <poll.h>

#include <errno.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>

#include <Client.h>
#include <RParser.h>
#include <Channel.h>
#include <Server.h>

#define	SET_COMMAND_FUNC(m, f)	_commandFuncs[m] = &Server::_command##f;

Server::Server(int port, str password): _flag(IRC_STATUS_OK), _port(port), _password(password) 
{
	IRC_LOG("Server constructor called.");

	_sockfd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
	if (_sockfd == -1)
		throw ServerSocketFailedException();

	_seeker.rebuild("");

	SET_COMMAND_FUNC("CAP", CAP);
	SET_COMMAND_FUNC("PASS", PASS);
	SET_COMMAND_FUNC("NICK", NICK);
	SET_COMMAND_FUNC("USER", USER);
	SET_COMMAND_FUNC("JOIN", JOIN);
	SET_COMMAND_FUNC("MODE", MODE);
	SET_COMMAND_FUNC("QUIT", QUIT);

	IRC_OK("socket opened on fd "BOLD(COLOR(GRAY,"[%d]")), _sockfd);
}

Server::~Server(void)
{
	IRC_LOG("Server destructor called.");

	for (IRC_AUTO it = _clients.begin(); it != _clients.end(); ++it)
	{
		if (it == _clients.begin())
			continue ;
		((*it).second).disconnect();
	}
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

void	Server::init()
{
	_bindSocket();
	_listenSocket();
	IRC_FLAG_SET(_flag, IRC_STATUS_OK);
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

void	Server::_connectClient(int fd)
{
	struct pollfd	client_pfd;
	
	client_pfd.fd = fd;
	client_pfd.events = POLLIN | POLLOUT;
	client_pfd.revents = 0;

	_pollSet.push_back(client_pfd);
}

void	Server::_disconnectClient(Client *client)
{
	client->disconnect();
}

struct pollfd	*Server::_acceptClient()
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
		return (NULL);
	}
	_connectClient(fd);

	IRC_OK("client accepted on fd "BOLD(COLOR(GRAY,"[%d]"))".", fd);
	return (&_pollSet[_pollSet.size() - 1]);
}

#define R_NOAUTH_COMMANDS		R_START_STRING R_CAPTURE("CAP|PASS|USER|NICK|QUIT")
#define R_CHANNEL_COMMANDS		R_START_STRING R_CAPTURE("JOIN|MODE")
#define	R_IRC_ACCEPTED_COMMANDS	R_NOAUTH_COMMANDS "|" R_CHANNEL_COMMANDS

#define R_IRC_NOSPCRLFCL	R_CHAR_INV_GROUP(" \r\n:")
#define R_COMMAND_PASS		R_START_STRING R_1_OR_MORE(" ") R_CAPTURE(R_1_OR_MORE(R_IRC_NOSPCRLFCL))

#include <RplGenerator.h>

/**
 * @brief	PASS command
 *
 * can reply 461 462 or 463  
 */
IRC_COMMAND_DEF(PASS)
{
	UNUSED(command);
	uint32_t const	*flag = &client->get_flag();

	_seeker.rebuild(R_COMMAND_PASS);
	_seeker.capture(command, 1);

// 	RplGenerator		gen;
// 	std::vector<str>	&argv = _seeker.get_matches();
// 
// 	if (argv.size() == 0)
// 	{
// 		_send(client, gen.ERR_NEEDMOREPARAMS("", 2, "PASS", "Not enough parameters."));
// 		return ;
// 	}
// 	if (argv[1] != _password)
// 	{
// 		_send(client, gen.ERR_PASSWDMISMATCH("", 1, "Password incorrect."));
// 		return ;
// 	}

	// Already registered, so error.
	if (IRC_FLAG_GET(*flag, IRC_CLIENT_AUTH))
		;


	// replace last password for verification, only last is used
	//
	// iIRC_CLIENT_AUTH -> ERR_ALREADYREGISTERED(client.get_nickname())
	//
	//
}


IRC_COMMAND_DEF(CAP)
{
	UNUSED(command);
	UNUSED(client);
}

IRC_COMMAND_DEF(NICK)
{
	UNUSED(command);
	UNUSED(client);

//	_send(client, "NICK rgramati");
}

IRC_COMMAND_DEF(USER)
{
	UNUSED(command);
	UNUSED(client);
}

IRC_COMMAND_DEF(JOIN)
{
	UNUSED(command);
	UNUSED(client);
}

IRC_COMMAND_DEF(MODE)
{
	UNUSED(command);
	UNUSED(client);
}

IRC_COMMAND_DEF(QUIT)
{
	UNUSED(command);
	UNUSED(client);
	client->disconnect();
}

#define R_COMMAND_SPLIT	R_CAPTURE(R_1_OR_MORE(R_IRC_NOSPCRLFCL))

void	Server::_executeCommand(Client *client, const str &command)
{
	_seeker.rebuild(R_COMMAND_SPLIT);
	_seeker.findall(command);

	const std::vector<str>	&argv = _seeker.get_matches();
	if (argv.size() < 1)
		return ;

	_seeker.rebuild(R_IRC_ACCEPTED_COMMANDS);

	if (!_seeker.match(command))
		return ;

	IRC_LOG(BOLD(COLOR(CYAN,"command execution : <%s>")), command.c_str());

	const str	mnemo = argv[0];
	(this->*_commandFuncs[mnemo])(client, command.substr(mnemo.length(), command.length()));
}

void	Server::_handleMessage(Client *client)
{
	str					buffer = client->get_buffer();
	std::vector<str>	commands;

	while (true)
	{
		const size_t	linefeed = buffer.find("\n");
		if (!linefeed || linefeed == std::string::npos)
			break ;
		const size_t	crlf = (buffer.at(linefeed - 1) == '\r');
		const str		command = buffer.substr(0, linefeed - crlf);
	
		commands.push_back(command);

		buffer = buffer.substr(linefeed + 1, buffer.length());
	}
	client->resetBuffer();

	bool	isAuth = IRC_FLAG_GET(client->get_flag(), IRC_CLIENT_AUTH);

	_seeker.rebuild(R_NOAUTH_COMMANDS);
	for (IRC_AUTO it = commands.begin(); it != commands.end(); ++it)
	{
		IRC_WARN("Calling execution on " BOLD(COLOR(YELLOW, "%s")), (*it).c_str());
		if (!isAuth && !_seeker.match(*it))
			continue ;
		_executeCommand(client, *it);
	}
}

void	Server::start()
{
	IRC_LOG("server started, polling...");

	struct pollfd	server_pfd;

	server_pfd.fd = _sockfd;
	server_pfd.events = POLLIN | POLLOUT;
	_pollSet.push_back(server_pfd);

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

					if (!client_pfd)
						continue ;
					_clients[client_pfd->fd] = Client(client_pfd, 0);
				}
				else
				{
					client = &_clients[curr.fd];

					client->readBytes();
					if (IRC_FLAG_GET(client->get_flag(), IRC_CLIENT_EOT))
					{
						IRC_WARN("Total message = \n" BOLD(COLOR(RED,"%s")), client->get_buffer().c_str());
						_handleMessage(client);
					}
					if (!IRC_FLAG_GET(client->get_flag(), IRC_CLIENT_AUTH))
						continue ;
					if (IRC_FLAG_GET(client->get_flag(), IRC_CLIENT_EOF))
						_disconnectClient(client);
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
