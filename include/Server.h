#pragma once

#include "bot/blackjack.h"
# include <unistd.h>
# include <sys/types.h>
# include <sys/socket.h>
# include <errno.h>
# include <netdb.h>
# include <map>

# include <irc.h>
# include <Client.h>
# include <Channel.h>
# include <IRCSeeker.h>
# include <IRCArchitect.h>

# define COMMA	,

typedef enum:	uint32_t
{
	IRC_STATUS_NULL	=	0U,
	IRC_STATUS_OK	=	1U << 0,
}	serverFlag;

extern volatile	bool	interrupt;

class	Server;

# define	IRC_COMMAND_DECL(mnemonic)	void	_command##mnemonic(Client &client, const str &command)
# define	IRC_COMMAND_DEF(mnemonic)	void	Server::_command##mnemonic(Client &client, const str &command)
# define	IRC_COMMAND_FUNC(m, f)		_commandFuncs[m] = &Server::_command##f;

typedef	void	(Server::*IRC_COMMAND_F)(Client &, const str &command);

# define	IRC_SERVER_RUNNING			!interrupt && IRC_FLAG_GET(_flag, IRC_STATUS_OK)
# define	IRC_POLLIN(t)				(t.revents & POLLIN)

# define	IRC_CLIENT_INCOMING(s)		IRC_POLLIN(s)
# define	IRC_CLIENT_WRITING(c)		IRC_POLLIN(c)
# define	IRC_CLIENT_OFFLINE(c)		IRC_FLAG_GET((c).get_flag(), IRC_CLIENT_EOF)
# define	IRC_CLIENT_PENDING(c)		IRC_FLAG_GET((c).get_flag(), IRC_CLIENT_EOT)

# define IRC_NICKLEN	9

class Server
{
	private:
		uint32_t					_flag;
		IRCSeeker					_seeker;
		IRCArchitect				_architect;

		std::map<str, Channel *>	_channelMap;

		/**
		 * @ Server config
		 */
		int							_port;
		int							_sockfd;
		str							_hostname;
		str							_password;
		time_t						_startTime;

		void	_bindSocket()
		{	
			IRC_LOG("binding...");

			sockaddr_in	addr;

			addr.sin_family = AF_INET;
			addr.sin_port = htons(_port);
			addr.sin_addr.s_addr = INADDR_ANY;

			int err = bind(_sockfd, (struct sockaddr *)&addr, sizeof(addr));
			if (err)
				throw ServerBindFailedException();

			IRC_OK("socket bound to port %d.", _port);
		}

		void	_listenSocket()
		{
			IRC_LOG("listening...");

			int err = listen(_sockfd, 10);
			if (err)
				throw ServerListenFailedException();

			IRC_OK("listening on port %d.", _port);
		}

		/**
		 * @ Poll utilities
		 */
		std::vector<struct pollfd>		_pollSet;

		void	_addPollFd(int fd)
		{
			struct pollfd	pfd = 
			{
				.fd = fd,
				.events = POLLIN,
				.revents = 0,
			};

			_pollSet.push_back(pfd);
		}

		void	_delPollFd(int fd)
		{
			for (size_t id = 0; id < _pollSet.size(); ++id)
			{
				if (_pollSet[id].fd == fd)
				{
					_pollSet.erase(_pollSet.begin() + id);
					return ;
				}
			}
		}

		bool	_updatePollSet()
		{
			int err = poll(&_pollSet[0], _pollSet.size(), 1);
			if (err < 0)
			{
				if (errno == EAGAIN)
					IRC_ERR("poll failed " COLOR(RED,"[EAGAIN]") ", trying again...");
				if (errno == EAGAIN || errno == EINTR)
					return (true);
				throw ServerPollFailedException();
			}
			return (!err);
		}
	
		/**
		 * @ Client global handling
		 */
		std::map<int, Client>			_clients;

		int	_acceptClient(void)
		{
			IRC_LOG("new client connection attempt.");

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
			if (_clients.size() == IRC_CLIENT_CAP)
			{
				IRC_WARN("too many clients, connection refused.");
				close(fd);
				return (-1);
			}
			return (fd);
		}

		void	_connectClient(void)
		{
			int32_t	client_fd = _acceptClient();
			if (client_fd == -1)
				return ;

			IRC_LOG("client " BOLD(COLOR(GRAY,"[%d]")) " connected.", client_fd);

			_clients[client_fd] = Client(0, client_fd);
			_addPollFd(client_fd);
		}

		void	_disconnectClient(Client &client)
		{
			int fd = client.get_fd();

			IRC_LOG("client " BOLD(COLOR(GRAY,"[%d]")) " disconnected.", fd);

			for (IRC_AUTO it = _channelMap.begin(); it != _channelMap.end(); ++it)
			{
				IRC_AUTO	chan = it->second;
				IRC_AUTO	&clientMap = chan->get_clientsMap();
				IRC_AUTO	clientIt = clientMap.find(fd);

				if (clientIt != clientMap.end())
				{
					IRC_FLAG_SET(clientIt->second, IRC_CHANNEL_IGNORED);
					if (IRC_FLAG_GET(clientIt->second, IRC_CHANNEL_INVITED))
						chan->removeClient(fd);
					else
						_commandPART(client, str("PART ") + chan->getTargetName());
				}
			}
			client.disconnect();
			_clients.erase(fd);
			_delPollFd(fd);
		}

		void	_welcomeClient(Client &client)
		{
			const char	*nickname = client.get_nickname().c_str();
			const str	created = str(RPL_MSG_CREATED) + str(ctime(&_startTime));

			_send(client, _architect.RPL_WELCOME (nickname, (RPL_MSG_WELCOME + client.get_nickname()).c_str()));
			_send(client, _architect.RPL_YOURHOST(nickname));
			_send(client, _architect.RPL_CREATED(nickname, created.substr(0, created.length() - 1).c_str()));
			_send(client, _architect.RPL_MYINFO(nickname, "ft_irc", "0.0", "o", "ikl"));
			_send(client, _architect.RPL_ISUPPORT(nickname, "NICKLEN=9"));
			_send(client, _architect.ERR_NOMOTD(nickname));
		}

		void	_registerClient(Client &client)
		{
			if (client.get_lastPass() != _password)
			{
				_send(client, _architect.ERR_PASSWDMISMATCH(client.get_nickname().c_str()));
				_send(client, "ERROR");
				_disconnectClient(client);
				return ;
			}

			_send(client, "PING ft_irc");
			IRC_FLAG_SET(client.get_flag(), IRC_CLIENT_PINGED);
		}

		/**
		 * @ Commands
		 */
		std::map<str, IRC_COMMAND_F>	_commandFuncs;

		void	_executeCommand(Client &client, const str &command)
		{
			_seeker.feedString(command);
			_seeker.rebuild(R_COMMAND_MNEMO);
			if (!_seeker.consume())
			{
				IRC_WARN(IRC_SEEKER "invalid command mnemonic.");
				return ;
			}

			std::vector<str>	&argv = _seeker.get_matches();
			const str			strip = _seeker.get_string();	
			const str			mnemo = argv[0];
			IRC_AUTO			func = _commandFuncs.find(mnemo);

			if (func == _commandFuncs.end())
			{
				IRC_WARN(IRC_SEEKER "command not supported.");
				return ;
			}

			_seeker.feedString(mnemo);
			if (!(client.get_flag() & IRC_CLIENT_AUTH))
			{
				_seeker.rebuild(R_IRC_NOAUTH_COMMANDS);
				if (!_seeker.consume())
				{
					IRC_WARN(IRC_SEEKER "ignoring non-authentified client command.");
					return ;
				}
			}
			
			IRC_LOG(IRC_ANALYST "calling function : " BOLD(COLOR(GRAY,"[%s]")) ":" BOLD(COLOR(GRAY,"{%s}")) , mnemo.c_str(), strip.c_str());

			(this->*(func->second))(client, strip);
		}

		void	_handleMessage(Client &client)
		{
			str					buffer = client.get_buffer();
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
			client.resetBuffer();

			for (IRC_AUTO it = commands.begin(); it != commands.end(); ++it)
			{
				if (IRC_FLAG_GET(client.get_flag(), IRC_CLIENT_EOF))
					break ;
				_executeCommand(client, *it);
			}
		}

		void	_send(Client &client, const str &string)
		{
			client.sendMsg(":" + _hostname + " " + string);
		}

		void	_broadcast(const str &string)
		{
			IRC_AUTO	it = _clients.begin();

			IRC_LOG("Server Brodcast " BOLD(COLOR(YELLOW,"%s")), string.c_str());
			for (; it != _clients.end(); ++it)
				_send((*it).second, string);
		}

		void	_sendJoin(Client &client, Channel *channel);
		void	_sendTopic(Client &client, Channel *channel);
		void	_sendModeIs(Client &client, Channel *channel);

		IRC_COMMAND_DECL(PASS);
		IRC_COMMAND_DECL(NICK);
		IRC_COMMAND_DECL(USER);
		IRC_COMMAND_DECL(PING);
		IRC_COMMAND_DECL(PONG);
		IRC_COMMAND_DECL(JOIN);
		IRC_COMMAND_DECL(KICK);
		IRC_COMMAND_DECL(PRIVMSG);
		IRC_COMMAND_DECL(INVITE);
		IRC_COMMAND_DECL(TOPIC);
		IRC_COMMAND_DECL(PART);
		IRC_COMMAND_DECL(MODE);
		IRC_COMMAND_DECL(QUIT);

		void	_UserJoinChannel(const str &, const str *, Client &);
		void	_kickUserFromChannel(str , Client &, str , str *);
		bool	_individualMode(bool, char, const str &, Channel *, Client &);
		
		Client	*_getClientByName(const str userName)
		{
			for (IRC_AUTO it = _clients.begin(); it != _clients.end(); ++it)
			{
				if (it->second.get_nickname() == userName)
					return &it->second;
			}
			return NULL;
		}

		Channel	*_getChannelByName(const str Name)
		{
			IRC_AUTO it = _channelMap.find(Name);
			if (it != _channelMap.end())
				return (*it).second;
			return NULL;
		}

		ATarget	*_getTargetByName(const str Name)
		{
			ATarget *target;

			if (!(target = _getClientByName(Name)))
				target = _getChannelByName(Name);

			return target;
		}

	public:

		Server(int port, str password): _flag(IRC_STATUS_OK), _port(port)
		{
			struct addrinfo	hints;
			struct addrinfo	*result;

			IRC_BZERO(hints);
			hints.ai_family = AF_INET;
			hints.ai_socktype = SOCK_STREAM;
			hints.ai_flags = AI_CANONNAME;

			int err = getaddrinfo("localhost", NULL, &hints, &result);
			if (err)
				throw AddrinfoFailedException();
			_hostname = result->ai_canonname;
			freeaddrinfo(result);

			_sockfd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
			if (_sockfd == -1)
				throw ServerSocketFailedException();
			_addPollFd(_sockfd);	

			_seeker.feedString(" " + password);
 			_seeker.rebuild(R_MIDDLE_PARAM);
			if (!_seeker.consume())
				throw InvalidPasswordException();
			_password = password;

			time(&_startTime);

			IRC_COMMAND_FUNC("PASS", PASS);

			IRC_COMMAND_FUNC("NICK", NICK);
			IRC_COMMAND_FUNC("USER", USER);
			IRC_COMMAND_FUNC("PING", PING);
			IRC_COMMAND_FUNC("PONG", PONG);
			IRC_COMMAND_FUNC("PRIVMSG", PRIVMSG);
			IRC_COMMAND_FUNC("INVITE", INVITE);
			IRC_COMMAND_FUNC("TOPIC", TOPIC);
			IRC_COMMAND_FUNC("QUIT", QUIT);
			IRC_COMMAND_FUNC("PART", PART);
			IRC_COMMAND_FUNC("JOIN", JOIN);
			IRC_COMMAND_FUNC("KICK", KICK);
			IRC_COMMAND_FUNC("MODE", MODE);
			IRC_COMMAND_FUNC("BJ", BJ);

			IRC_OK("ft_irc@%s server " BOLD(COLOR(GRAY,"[%d]")) " started.", _hostname.c_str(), _sockfd);
		}

		IRC_COMMAND_DECL(BJ)
		{
			UNUSED(command);
			BlackJack	game;

			game.addPlayer(3);
			game.addPlayer(4);

			str gamestr = game.displayGame();

			_send(client, gamestr);
		}

		~Server(void)
		{
			while (_clients.size())
			{
				Client	&client = (*(_clients.begin())).second;
				_disconnectClient(client);
			}
			if (_sockfd > STDERR_FILENO)
				close(_sockfd);

			IRC_OK("ft_irc@%s server " BOLD(COLOR(GRAY,"[%d]")) " stopped.", _hostname.c_str(), _sockfd);
		}

		void	init(void)
		{
			_bindSocket();
			_listenSocket();
			IRC_FLAG_SET(_flag, IRC_STATUS_OK);
		}

		void	start()
		{
			IRC_LOG("server started.");

			while (IRC_SERVER_RUNNING)
			{
				if (_updatePollSet())
					continue ;

				struct pollfd	server = _pollSet[0];

				if (IRC_CLIENT_INCOMING(server))
					_connectClient();

				for (size_t id = 1; id < _pollSet.size(); ++id)
				{
					struct pollfd	curr = _pollSet[id];

					if (!IRC_CLIENT_WRITING(curr))
						continue ;

					Client	&client = _clients[curr.fd];

					client.readBytes();
					if (IRC_CLIENT_PENDING(client))
						_handleMessage(client);
					if (IRC_CLIENT_OFFLINE(client))
					{
						_disconnectClient(client);
						break ;
					}
				}
			}
		}

		GETTER(uint32_t, _flag);
		GETTER(int, _port);
		GETTER(int, _sockfd);
		GETTER(std::map<int COMMA Client>, _clients);

		GETTER_C(uint32_t, _flag);
		GETTER_C(int, _port);
		GETTER_C(int, _sockfd);
		GETTER_C(std::map<int COMMA Client>, _clients);

		SETTER(uint32_t, _flag);
		SETTER(int, _port);
		SETTER(int, _sockfd);

	EXCEPTION(AddrinfoFailedException,		"getaddrinfo() failed.");
	EXCEPTION(InvalidPasswordException,		"invalid password.");
	EXCEPTION(ServerSocketFailedException,	"socket() failed.");
	EXCEPTION(ServerBindFailedException,	"bind() failed.");
	EXCEPTION(ServerListenFailedException,	"listen() failed.");
	EXCEPTION(ServerPollFailedException,	"poll() failed.");
	EXCEPTION(ServerAcceptFailedException,	"accept() failed.");
};

# include <commands/pass.h>
# include <commands/nick.h>
# include <commands/user.h>

# include <commands/pong.h>
# include <commands/ping.h>

# include <commands/join.h>
# include <commands/part.h>
# include <commands/topic.h>
# include <commands/invite.h>
# include <commands/kick.h>
# include <commands/mode.h>
# include <commands/privmsg.h>

# include <commands/quit.h>
