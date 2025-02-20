/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rgramati <rgramati@42angouleme.fr>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/02/20 20:04:51 by rgramati          #+#    #+#             */
/*   Updated: 2025/02/20 20:07:18 by rgramati         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "IRCSeeker.h"
#include "ircRegex.h"
#include <poll.h>

#include <errno.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>

#include <Server.h>
#include <Client.h>

#define	SET_COMMAND_FUNC(m, f)	_commandFuncs[m] = &Server::_command##f;

Server::Server(int port, str password): _flag(IRC_STATUS_OK), _port(port), _password(password) 
{
	IRC_LOG("Server constructor called.");

	_sockfd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
	if (_sockfd == -1)
		throw ServerSocketFailedException();

	SET_COMMAND_FUNC("PASS", PASS);
	SET_COMMAND_FUNC("NICK", NICK);
	SET_COMMAND_FUNC("USER", USER);
	SET_COMMAND_FUNC("PONG", PONG);
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

struct pollfd	*Server::_acceptClient(void)
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

void	Server::_welcomeRoutine(void)
{

}

#define IRC_NO_PREFIX	""

#define R_NOAUTH_COMMANDS		R_START_STRING R_CAPTURE("PASS|USER|NICK|QUIT")
#define R_CHANNEL_COMMANDS		R_START_STRING R_CAPTURE("JOIN|MODE")
#define	R_IRC_ACCEPTED_COMMANDS	R_NOAUTH_COMMANDS "|" R_CHANNEL_COMMANDS

#define	R_SPACE				" "
#define R_NOCRLF			R_CHAR_INV_GROUP("\r\n")
#define R_NOSPCRLFCL		R_CHAR_INV_GROUP("\r\n :")

#define R_MIDDLE			R_NOSPCRLFCL R_0_OR_MORE(R_CAPTURE(":|" R_NOSPCRLFCL))
#define R_MIDDLE_PARAM		R_SPACE R_0_OR_1(R_CAPTURE(R_MIDDLE))

#define R_TRAILING			R_0_OR_MORE(R_NOCRLF)
#define R_TRAILING_PARAM	R_SPACE	R_0_OR_1(R_CAPTURE(R_TRAILING))

IRC_COMMAND_DEF(PASS)
{
	_seeker.feedString(command);
	_seeker.rebuild(R_MIDDLE_PARAM);
	_seeker.findall();
	std::vector<str>	&argv = _seeker.get_matches();

	if (argv.size() == 0)
	{
		_send(client, 
		_architect.ERR_NEEDMOREPARAMS
		(
			IRC_NO_PREFIX, 3,
			client->get_nickname().c_str(),
			"PASS",
			"Not enough parameters"
		));
		return ;
	}

	if (IRC_FLAG_GET(client->get_flag(), IRC_CLIENT_AUTH))
	{
		_send(client, 
		_architect.ERR_ALREADYREGISTERED
		(
			IRC_NO_PREFIX, 2,
			client->get_nickname().c_str(),
			"You may not reregister"
		));
		return ;
	}
	client->set_flag(client->get_flag() | IRC_CLIENT_REGISTER);
	client->set_lastPass(argv[0]);
}

#define	NICKNAME_CHAR		" ,\\*\\?!@#"
#define NICKNAME_START		NICKNAME_CHAR ":$"
#define R_NICKNAME			R_FULL_MATCH											\
							(														\
								R_CAPTURE											\
								(													\
									R_CHAR_INV_GROUP(NICKNAME_START)				\
									R_X_TO_Y(R_CHAR_INV_GROUP(NICKNAME_CHAR),0,8)	\
								)													\
							)

IRC_COMMAND_DEF(NICK)
{
	_seeker.feedString(command);
	_seeker.rebuild(R_MIDDLE_PARAM);
	_seeker.findall();
	const std::vector<str>	&argv = _seeker.get_matches();

	if (argv.size() == 0)
	{
		_send(client, 
		_architect.ERR_NONICKNAMEGIVEN 
		(
			IRC_NO_PREFIX, 2,
			client->get_nickname().c_str(),
			"No nickname given"
		));
		return ;
	}

	const str	&newNick = argv[0];
	_seeker.feedString(newNick);
	_seeker.rebuild(R_NICKNAME);

	if (!_seeker.consume())
	{
		_send(client,
		_architect.ERR_ERRONEUSNICKNAME
		(
			IRC_NO_PREFIX, 3,
			client->get_nickname().c_str(),
			newNick.c_str(),
			"Erroneus nickname"
		));
		return ;
	}

	for (IRC_AUTO it = _clients.begin(); it != _clients.end(); ++it)
	{
		if (newNick == (*it).second.get_nickname())
		{
			_send(client,
			_architect.ERR_NICKNAMEINUSE
			(
				IRC_NO_PREFIX, 3,
				client->get_nickname().c_str(),
				newNick.c_str(),
				"Nickname is already in use"
			));
			return ;
		}
	}
	client->set_nickname(newNick);
	if (client->get_username() == "")
		client->set_username(newNick);
}

#define R_USERNAME	R_FULL_MATCH										\
					(													\
						R_CAPTURE										\
						(												\
							R_1_OR_MORE(R_CHAR_INV_GROUP("\r\n @"))		\
						)												\
					)

void	Server::_registerClient(Client *client)
{
	if (client->get_lastPass() != _password)
	{
		_send(client,
		_architect.ERR_PASSWDMISMATCH
		(
			IRC_NO_PREFIX, 2,
			client->get_nickname().c_str(),
			"Password incorrect"
		));
		return ;
	}
	_send(client, "PING ft_irc");
	client->set_flag(client->get_flag() | IRC_CLIENT_PINGED);
}

IRC_COMMAND_DEF(USER)
{
	_seeker.feedString(command);
	_seeker.rebuild(R_MIDDLE_PARAM);
	_seeker.findall();
	std::vector<str>	&argv = _seeker.get_matches();

	if (argv.size() != 3)
	{
		_send(client, 
		_architect.ERR_NEEDMOREPARAMS
		(
			IRC_NO_PREFIX, 3,
			client->get_nickname().c_str(),
			"USER",
			"Not enough parameters"
		));
		return ;
	}

	if (IRC_FLAG_GET(client->get_flag(), IRC_CLIENT_AUTH))
	{
		_send(client, 
		_architect.ERR_ALREADYREGISTERED
		(
			IRC_NO_PREFIX, 2,
			client->get_nickname().c_str(),
			"You may not reregister"
		));
		return ;
	}

	const str	&newUser = argv[0];

	_seeker.feedString(newUser);
	_seeker.rebuild(R_USERNAME);
	if (!_seeker.consume())
	{
		_send(client,
		_architect.ERR_NEEDMOREPARAMS
		(
			IRC_NO_PREFIX, 3,
			client->get_nickname().c_str(),
			"USER",
			"Not enough parameters"
		));
		return ;
	}
	client->set_username(newUser);
	_registerClient(client);
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

#define IRC_CAN_PONG	(IRC_CLIENT_PINGED | IRC_CLIENT_REGISTER)

IRC_COMMAND_DEF(PONG)
{
	if ((client->get_flag() & IRC_CAN_PONG) != IRC_CAN_PONG)
		return ;
	client->set_flag(client->get_flag() & ~(IRC_CLIENT_PINGED));

	_seeker.feedString(command);
	_seeker.rebuild(R_MIDDLE_PARAM);
	if (!_seeker.consume())
		return ;

	IRC_LOG("parameter found");

	const std::vector<str>	&argv = _seeker.get_matches();
	if (argv.size() != 1)
		return ;
	
	IRC_LOG("1 argument");

	if (argv[0] != "ft_irc")
		return ;

	IRC_LOG("good token");
	
	client->set_flag(client->get_flag() | IRC_CLIENT_AUTH);
	client->set_flag(client->get_flag() & ~(IRC_CLIENT_REGISTER));

	_welcomeRoutine();
}

#define R_COMMAND_MNEMO			R_CAPTURE_WORD

#define IRC_NOAUTH_COMMANDS		"PASS|NICK|USER|PONG|QUIT"
#define IRC_VALID_COMMANDS		IRC_NOAUTH_COMMANDS "|JOIN|MODE"

#define R_IRC_NOAUTH_COMMANDS	R_FULL_MATCH(R_CAPTURE(IRC_NOAUTH_COMMANDS))
#define R_IRC_VALID_COMMANDS	R_FULL_MATCH(R_CAPTURE(IRC_VALID_COMMANDS))

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
// 		bool	isAuth = IRC_FLAG_GET(client->get_flag(), IRC_CLIENT_AUTH);

// 		_seeker.rebuild(R_NOAUTH_COMMANDS);
// 		_seeker.feedString(*it);
// 		if (!isAuth && !_seeker.match())
// 		{
// 			IRC_WARN("COMMAND IGNORED" BOLD(COLOR(YELLOW, "[ %s ]")), (*it).c_str());
// 			continue ;
// 		}

		IRC_OK(BOLD(COLOR(GREEN, "EXECUTION CALL [ %s ]")), (*it).c_str());
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

					IRC_LOG("STARTING CLIENT FLAG = 0x%016X", client->get_flag());
					client->readBytes();
					if (IRC_FLAG_GET(client->get_flag(), IRC_CLIENT_EOT))
					{
						IRC_LOG("FULL MESSAGE = {\n" BOLD(COLOR(RED,"%s")) "\n}", client->get_buffer().c_str());
						_handleMessage(client);
					}
					if (IRC_FLAG_GET(client->get_flag(), IRC_CLIENT_EOF))
						_disconnectClient(client);
					IRC_LOG("NEW CLIENT FLAG = 0x%016X", client->get_flag());
					IRC_LOG("------------------------------------");
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
