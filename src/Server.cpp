
extern volatile bool	interrupt;

#include <unistd.h>
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
#include <ChannelJoin.h>

Server::Server(int port, str password): _flag(IRC_STATUS_OK), _port(port), _password(password) 
{
	IRC_LOG("Server constructor called.");

	_sockfd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
	if (_sockfd == -1)
		throw ServerSocketFailedException();

	IRC_COMMAND_FUNC("PASS", PASS);
	IRC_COMMAND_FUNC("NICK", NICK);
	IRC_COMMAND_FUNC("USER", USER);
	IRC_COMMAND_FUNC("PONG", PONG);
	IRC_COMMAND_FUNC("JOIN", JOIN);
	IRC_COMMAND_FUNC("MODE", MODE);
	IRC_COMMAND_FUNC("QUIT", QUIT);

	time_t		timestamp;

	time(&timestamp);
	_startTime = ctime(&timestamp);

	IRC_OK("socket opened on fd " BOLD(COLOR(GRAY,"[%d]")), _sockfd);
}

Server::~Server(void)
{
	IRC_LOG("Server destructor called.");

	for (IRC_AUTO it = _clients.begin(); it != _clients.end(); ++it)
	{
		Client	&client = (*it).second;
		client.disconnect();
	}
	close(_sockfd);

	IRC_OK("closed socket on fd " BOLD(COLOR(GRAY,"[%d]")), _sockfd);
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

void	Server::_listenSocket() const
{
	IRC_LOG("listening...");

	int err = listen(_sockfd, 10);
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
			_pollSet.erase(id);
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

	_pollSet.push(client_pfd);
}

void	Server::_disconnectClient(Client &client)
{
	IRC_LOG("disconnecting client, fd = " BOLD(COLOR(GRAY,"[%d]")), client.get_pfd()->fd);

	client.set_flag(client.get_flag() & ~IRC_CLIENT_EOF);
	client.disconnect();
	_clients.erase(client.get_pfd()->fd);
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
	IRC_OK("client accepted, fd = "BOLD(COLOR(GRAY,"[%d]"))".", fd);

	if (_clients.size() == IRC_CLIENT_CAP)
	{
		IRC_WARN("too many clients, connection refused.");
		close(fd);
		return (NULL);
	}
	_connectClient(fd);
	IRC_OK("client connected, fd = "BOLD(COLOR(GRAY,"[%d]"))".", fd);

	return (&_pollSet[_pollSet.size() - 1]);
}

void	Server::_welcomeClient(Client *client)
{
	const char	*username = client->get_username().c_str();

	_send(client, _architect.RPL_WELCOME (username, (RPL_MSG_WELCOME + client->get_username()).c_str()));
	_send(client, _architect.RPL_YOURHOST(username));
	_send(client, _architect.RPL_CREATED (username, (RPL_MSG_CREATED + _startTime).c_str()));
}

void	Server::_registerClient(Client *client)
{
	if (client->get_lastPass() != _password)
	{
		_send(client,
		_architect.ERR_PASSWDMISMATCH
		(
			client->get_nickname().c_str(),
			"Password incorrect"
		));
		return ;
	}
	_send(client, "PING ft_irc");
	client->set_flag(client->get_flag() | IRC_CLIENT_PINGED);
}

IRC_COMMAND_DEF(MODE)
{
	UNUSED(client);
	UNUSED(command);
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
	server_pfd.events = POLLIN | POLLOUT;
	_pollSet.push(server_pfd);

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
					struct pollfd	*client_pfd = _acceptClient();
					IRC_LOG("NEW CLIENT PFD = %p", client_pfd);

					if (!client_pfd)
						continue ;
					_clients[client_pfd->fd] = Client(client_pfd, 0);
				}
				else
				{
					client = &_clients[curr.fd];

					IRC_LOG("Treating with client " BOLD(COLOR(CYAN,"%p")) ", PFD at %p", client, client->get_pfd());
					client->readBytes();
					if (IRC_FLAG_GET(client->get_flag(), IRC_CLIENT_EOT))
					{
						IRC_LOG("FULL MESSAGE = {\n" BOLD(COLOR(RED,"%s")) "\n}", client->get_buffer().c_str());
						_handleMessage(client);
					}
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
