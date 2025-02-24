#ifndef CLIENT_HPP
# define CLIENT_HPP

# include <irc.h>
# include <map>

class Channel;

typedef enum:	uint32_t
{
	IRC_CLIENT_AUTH		=	1U,
	IRC_CLIENT_REGISTER	=	1U << 1,
	IRC_CLIENT_PINGED	=	1U << 2,
	IRC_CLIENT_EOT		=	1U << 30,
	IRC_CLIENT_EOF		=	1U << 31,
}	clientFlag;

#define COMMA	,

class Client
{
	private:
		uint32_t	_flag;
		str			_username;
		str			_nickname;

		str			_buffer;
		str			_lastPass;

		struct pollfd				*_pfd;
		std::map<str, Channel *>	_channelMap;

		int			_readToBuffer(void);

	public:
		Client(void);
		Client(struct pollfd *pfd, uint32_t flag);
		~Client(void);

		void	readBytes(void);
		void	resetBuffer(void);

		void	disconnect(void);
		void	joinChannel(Channel *);
		void	leaveChannel(Channel *);
		void	leaveChannel(Channel *, str);
		
		GETTER(uint32_t, _flag);
		GETTER(str, _username);
		GETTER(str, _nickname);
		GETTER(str, _buffer);
		GETTER(str, _lastPass);
		GETTER(struct pollfd *, _pfd);
		GETTER(std::map<str COMMA Channel *>, _channelMap);
		
		GETTER_C(uint32_t, _flag);
		GETTER_C(str, _username);
		GETTER_C(str, _nickname);
		GETTER_C(str, _buffer);
		GETTER_C(str, _lastPass);
 		GETTER_C(std::map<str COMMA Channel *>, _channelMap);

		SETTER(uint32_t, _flag);
		SETTER(str, _username);
		SETTER(str, _nickname);
		SETTER(str, _lastPass);
};

#endif // CLIENT_HPP
