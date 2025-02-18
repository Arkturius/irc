#ifndef SERVER_HPP
# define SERVER_HPP

# include <sys/socket.h>
# include <map>
# include <vector>

# include <irc.h>
# include <IRCSeeker.h>
# include <IRCArchitect.h>

# define COMMA	,

typedef enum:	uint32_t
{
	IRC_STATUS_NULL	=	0U,
	IRC_STATUS_OK	=	1U << 0,
}	serverFlag;

class	Client;
class	Channel;
class	Server;

#define	IRC_COMMAND_DECL(mnemonic)	void	_command##mnemonic(Client *client, const str &command)
#define	IRC_COMMAND_DEF(mnemonic)	void	Server::_command##mnemonic(Client *client, const str &command)

typedef	void	(Server::*IRC_COMMAND_F)(Client *, const str &command);

class Server
{
	private:
		uint32_t	_flag;

		int		_port;
		int		_sockfd;
		str		_password;

		std::vector<struct pollfd>		_pollSet;
		std::map<int, Client>			_clients;

		std::map<str, IRC_COMMAND_F>	_commandFuncs;
		IRCSeeker						_seeker;
		IRCArchitect					_architect;

		void			_bindSocket() const;
		void			_listenSocket() const;

		bool			_updatePollSet();

		struct pollfd	*_acceptClient();
		void			_connectClient(int fd);
		void			_disconnectClient(Client *client);

		void			_handleMessage(Client *client);
		str				_extractCommand(str *source);

		void			_executeCommand(Client *client, const str &command);

		void			_send(Client *client, const str &string);
		void			_sendJoin(Client *client, Channel *channel);
		void			_sendTopic(Client *client, Channel *channel);
		void			_sendModeIs(Client *client, Channel *channel);

		IRC_COMMAND_DECL(CAP);
		IRC_COMMAND_DECL(PASS);
		IRC_COMMAND_DECL(USER);
		IRC_COMMAND_DECL(NICK);
		IRC_COMMAND_DECL(JOIN);
		IRC_COMMAND_DECL(MODE);
		IRC_COMMAND_DECL(QUIT);

	public:
		Server(int port, str password);
		~Server(void);

		void	init();
		void	start();

		void	serverInfo() const;

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

		EXCEPTION(ServerSocketFailedException,	"socket() failed.");
		EXCEPTION(ServerBindFailedException,	"bind() failed.");
		EXCEPTION(ServerListenFailedException,	"listen() failed.");
		EXCEPTION(ServerPollFailedException,	"poll() failed.");
		EXCEPTION(ServerAcceptFailedException,	"accept() failed.");
	
	private:
		std::map<str, Channel *>	_channelMap;
		void						_joinAddAllChannel(std::vector<str>, std::vector<str>, Client *);
		void						_addChannel(const str &channelName, const str *key, Client *);
		void						_removeChannel(str channelName, Client *);
		void						_kickChannel(str channelName, Client *admin, str kicked, str *comment);

		void						_join(const str &, Client *);
		void						_kick(const str, Client *);
		void						_topic(const str, Client *);
		void						_invite(const str, Client *);
		void						_mode(const str, Client *);
		bool						_individualMode(bool, char, const str &, Channel *, Client *);
		Client						*_getClientByName(const str);
		Channel						*_getChannelByName(const str);
		EXCEPTION(UnexpectedErrorException,	"oops");
};

#endif // SERVER_HPP
