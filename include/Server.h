#ifndef SERVER_HPP
# define SERVER_HPP

# include "ATarget.h"
# include "Channel.h"
# include <unistd.h>
# include <sys/types.h>
# include <sys/socket.h>
# include <netdb.h>

# include <map>

# include <irc.h>
# include <Client.h>
# include <IRCSeeker.h>
# include <IRCArchitect.h>

# define COMMA	,

typedef enum:	uint32_t
{
	IRC_STATUS_NULL	=	0U,
	IRC_STATUS_OK	=	1U << 0,
}	serverFlag;

class	Server;

#define	IRC_COMMAND_DECL(mnemonic)	void	_command##mnemonic(Client *client, const str &command)
#define	IRC_COMMAND_DEF(mnemonic)	void	Server::_command##mnemonic(Client *client, const str &command)
#define	IRC_COMMAND_FUNC(m, f)		_commandFuncs[m] = &Server::_command##f;

typedef	void	(Server::*IRC_COMMAND_F)(Client *, const str &command);

#define IRC_NICKLEN	9

class Server
{
	private:
		uint32_t	_flag;

		int			_port;
		int			_sockfd;

		str			_hostname;
		str			_password;
		time_t		_startTime;

		std::vector<struct pollfd>		_pollSet;
		std::map<int, Client>			_clients;

		std::map<str, IRC_COMMAND_F>	_commandFuncs;
		IRCSeeker						_seeker;
		IRCArchitect					_architect;

		void			_bindSocket(void) const;
		void			_listenSocket(void) const;

		bool			_updatePollSet(void);

		int32_t			_acceptClient(void);
		void			_connectClient(int fd);
		void			_registerClient(Client *client);
		void			_welcomeClient(Client *client);
		void			_disconnectClient(Client &client);

		void			_handleMessage(Client *client);
		str				_extractCommand(str *source);
		void			_executeCommand(Client *client, const str &command);

		void			_send(Client *client, const str &string);
		void			_broadcast(const str &string);
		void			_sendJoin(Client *client, Channel *channel);
		void			_sendTopic(Client *client, Channel *channel);
		void			_sendModeIs(Client *client, Channel *channel);

		IRC_COMMAND_DECL(PASS);
		IRC_COMMAND_DECL(NICK);
		IRC_COMMAND_DECL(USER);
		IRC_COMMAND_DECL(PONG);
		IRC_COMMAND_DECL(JOIN);
		IRC_COMMAND_DECL(KICK);
		IRC_COMMAND_DECL(PRIVMSG);
		IRC_COMMAND_DECL(INVITE);
		IRC_COMMAND_DECL(TOPIC);
		IRC_COMMAND_DECL(PART);
		IRC_COMMAND_DECL(MODE);
		IRC_COMMAND_DECL(QUIT);

	public:

		Server(int port, str password): _flag(IRC_STATUS_OK), _port(port)
		{
			IRC_LOG("Server constructor called.");

			struct addrinfo	hints;
			struct addrinfo	*result;

			IRC_BZERO(hints);
			hints.ai_family = AF_INET;
			hints.ai_socktype = SOCK_STREAM;
			hints.ai_flags = AI_CANONNAME;

			int err = getaddrinfo("localhost", NULL, &hints, &result);
			if (err)				{ throw AddrinfoFailedException();		}
			_hostname = result->ai_canonname;
			freeaddrinfo(result);

			_sockfd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
			if (_sockfd == -1)		{ throw ServerSocketFailedException();	}

			_seeker.feedString(" " + password);
 			_seeker.rebuild(R_MIDDLE_PARAM);
			if (!_seeker.consume())	{ throw InvalidPasswordException();		}
			_password = password;
	
			time(&_startTime);

			IRC_COMMAND_FUNC("PASS", PASS);
			IRC_COMMAND_FUNC("NICK", NICK);
			IRC_COMMAND_FUNC("USER", USER);
			IRC_COMMAND_FUNC("PONG", PONG);
			IRC_COMMAND_FUNC("JOIN", JOIN);
			IRC_COMMAND_FUNC("KICK", KICK);
			IRC_COMMAND_FUNC("PRIVMSG", PRIVMSG);
			IRC_COMMAND_FUNC("INVITE", INVITE);
			IRC_COMMAND_FUNC("TOPIC", TOPIC);
			IRC_COMMAND_FUNC("PART", PART);
			IRC_COMMAND_FUNC("MODE", MODE);
			IRC_COMMAND_FUNC("QUIT", QUIT);

			IRC_OK("socket [%s] opened on fd " BOLD(COLOR(GRAY,"[%d]")), _hostname.c_str(), _sockfd);
		}

		~Server(void)
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

		void	init(void)
		{
			_bindSocket();
			_listenSocket();
			IRC_FLAG_SET(_flag, IRC_STATUS_OK);
		}

		void	start();

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
	
	private:
		std::map<str, Channel *>	_channelMap;
		void						_joinAddAllChannel(std::vector<str> &, std::vector<str> &, Client *);
		void						_addChannel(const str &channelName, const str *key, Client *);
		void						_removeChannel(str channelName, Client *);
		void						_kickChannel(str channelName, Client *admin, str kicked, str *comment);
void								_kickAllChannel(std::vector<str> vecChannel, std::vector<str> vecUser, str *comment, Client *client);

		bool						modePassword(bool plus, const str &modeArguments, Channel *target, Client *client);
		bool						modePermition(bool plus, const str &modeArguments, Channel *target, Client *client);
		void						modeCmdReturn(bool plus, const char &individualModeChar, Channel *target, Client *client);
		bool						_individualMode(bool, char, const str &, Channel *, Client *);

		EXCEPTION(UnexpectedErrorException,	"oops");
		
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
			IRC_LOG("Channel found at %p", it == _channelMap.end() ? NULL : &it);
			if (it != _channelMap.end())
			{
				IRC_WARN("%s : %p", (*it).first.c_str(), (*it).second);
				return (*it).second;
			}
			return NULL;
		}

		ATarget	*_getTargetByName(const str Name)
		{
			ATarget *target;

			if (!(target = _getClientByName(Name)))
				target = _getChannelByName(Name);

			return target;
		}

};

#endif // SERVER_HPP
