#include "IRCArchitect.h"
#include <poll.h>

#include <errno.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>

#include <Server.h>
#include <Client.h>

#define	SET_COMMAND_FUNC(m, f)	_commandFuncs[m] = &Server::_command##f;

#include "commands/pass.h"
#include "commands/nick.h"
#include "commands/user.h"
#include "commands/quit.h"
#include "commands/pong.h"

#include <ircCommands.h>
#include <ChannelJoin.h>

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
	// SET_COMMAND_FUNC("MODE", MODE);
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

void	Server::_welcomeClient(void)
{

}

void	Server::_registerClient(Client *client)
{
	if (client->get_lastPass() != _password)
	{
		_send(client,
		_architect.ERR_PASSWDMISMATCH
		(
			2,
			client->get_nickname().c_str(),
			"Password incorrect"
		));
		return ;
	}
	_send(client, "PING ft_irc");
	client->set_flag(client->get_flag() | IRC_CLIENT_PINGED);
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
