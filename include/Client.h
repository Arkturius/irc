#pragma once

# include <unistd.h>

# include <irc.h>
# include <map>
# include <Channel.h>

typedef enum:	uint32_t
{
	IRC_CLIENT_AUTH				=	1U,
	IRC_CLIENT_REGISTER			=	1U << 1,
	IRC_CLIENT_PINGED			=	1U << 2,

	IRC_CLIENT_BOT				=	1U << 15,
	
	IRC_CLIENT_EOT				=	1U << 30,
	IRC_CLIENT_EOF				=	1U << 31,
}	clientFlag;

#define COMMA	,

class BlackJack ;

class Client: public ATarget
{
	private:
		uint32_t	_flag;

		int32_t		_fd;

		str			_username;
		str			_nickname;
		str			_lastPass;

		str			_buffer;

		int	_read(void)
		{
			char	tmp[1024] = {0};
			int 	bytes = 0;

			bytes = read(_fd, tmp, sizeof(tmp) - 1);
			if (bytes)
			{
				tmp[bytes] = 0;
				_buffer += str(tmp);
			}
			return (bytes);
		}

		std::map<str, Channel *>	_channelMap;
		BlackJack					*_bjTable;

	public:
		Client(void): ATarget() {}
		~Client(void) {}
		Client(uint32_t flag, int32_t fd): ATarget(), _flag(flag), _fd(fd), _username(""), _nickname(""), _bjTable(0) {}

		void	readBytes(void)
		{
			int	bytes;

			bytes = _read();
			if (bytes == 0 || _buffer.length() == 0)
			{
				IRC_FLAG_SET(_flag, IRC_CLIENT_EOF);
				return ;
			}
			if (_buffer.find("\n") != std::string::npos)
			{
				IRC_FLAG_SET(_flag, IRC_CLIENT_EOT);
				return ;
			}
		}

		void	resetBuffer(void)
		{
			_buffer = "";
			IRC_FLAG_DEL(_flag, IRC_CLIENT_EOT);
		}

		void	disconnect()
		{
			close(_fd);
			_fd = -1;
		}

		void	joinChannel(Channel *channel)
		{
			str	channelName = channel->getTargetName();

			IRC_AUTO s = _channelMap.find(channelName);
			if (s == _channelMap.end())
				_channelMap[channelName] = channel;
		}

		void	leaveChannel(Channel *channel)
		{
			std::map<str, Channel * >::iterator s;

			s = _channelMap.find(channel->getTargetName());
			if (s != _channelMap.end())
			{
				if (s->second->get_size() == 0)
					delete s->second;
				_channelMap.erase(s);
			}
		}

		void	sendMsg(const str &string) const
		{
			write(_fd, (string + "\r\n").c_str(), string.size() + 2);
		}
		void	ignoredFlag(int, int32_t) {;}
		
		GETTER(uint32_t, _flag);
		GETTER(int32_t, _fd);
		GETTER(str, _username);
		GETTER(str, _nickname);
		GETTER(str, _lastPass);
		GETTER(str, _buffer);
		GETTER(std::map<str COMMA Channel *>, _channelMap);
		GETTER(BlackJack *, _bjTable);
		
		GETTER_C(uint32_t, _flag);
		GETTER_C(str, _username);
		GETTER_C(str, _nickname);
		GETTER_C(str, _lastPass);
		GETTER_C(str, _buffer);
 		GETTER_C(std::map<str COMMA Channel *>, _channelMap);

		SETTER(uint32_t, _flag);
		SETTER(str, _username);
		SETTER(str, _targetName);
		SETTER(str, _nickname);
		SETTER(str, _lastPass);
		SETTER(BlackJack *, _bjTable);
};
