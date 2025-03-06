#pragma once

# include <map>
# include <unistd.h>
# include <ircRegex.h>
# include <irc.h>
# include <time.h>
# include <ATarget.h>

typedef enum:	int32_t
{
	IRC_CHANNEL_OPERATOR		=	1U,
	IRC_CHANNEL_IGNORED			=	1U << 1,

	IRC_CHANNEL_INVITED			=	1U << 2,

	IRC_CHANNEL_INVITE_ONLY		=	1U << 3,
	IRC_CHANNEL_ACTIVE_PASSWORD	=	1U << 4,
	IRC_CHANNEL_TOPIC_SET		=	1U << 5,
	IRC_CHANNEL_TOPIC_PERM		=	1U << 6,

}	channelFlag;

class Client;

#define COMMA ,

class Channel: public ATarget
{
	private:
		str						_password;

		int						_userLimit;
		str						_topic;
		time_t					_topicSetTime;
		str						_topicSetterNickName;

		std::map<int, int32_t>	_clientsMap;
		int32_t				_flag;

		void	_addClient(int fdClient, int32_t flag)
		{
			_clientsMap[fdClient] = flag;
		}

		int32_t	_getClient(int fdClient)
		{
			IRC_AUTO	it = _clientsMap.find(fdClient);
			if (it == _clientsMap.end())
				return -1;
			return it->second;
		}

		void	_write(const int fd, const str &string) const
		{
			write(fd, string.c_str(), string.size());
		}
		void	_broadcast(const str &string) const
		{
			int			ignoredClient = 0;
			const str	newString = string + "\r\n";

			for (IRC_AUTO it = _clientsMap.begin(); it != _clientsMap.end(); ++it)
			{
				if (IRC_FLAG_GET(it->second, IRC_CHANNEL_IGNORED))
					ignoredClient++;
				else
					_write(it->first, newString);
			}
			IRC_LOG("Channel Brodcast " BOLD(COLOR(YELLOW,"%s")) " to %d client (%d) ignored", string.c_str(), get_size(), ignoredClient);
		}
	
	public:

		Channel(str channelName, int firstClient): ATarget(channelName), _userLimit(100), _flag(0)
		{
			IRC_LOG("Channel constructor called : |%s|", channelName.c_str());

			_addClient(firstClient, IRC_CHANNEL_OPERATOR);
		}

		~Channel(void)
		{
			IRC_LOG("Channel destructor called.");
		}

		void	addClient(int fdClient, const str *password)
		{
			IRC_AUTO	it = _clientsMap.find(fdClient);

			if (IRC_FLAG_GET(it->second, IRC_CHANNEL_INVITED))
				goto addClientLabel;

			if (IRC_FLAG_GET(_flag, IRC_CHANNEL_ACTIVE_PASSWORD) && (!password || *password != _password))
				throw InvalidChannelKeyException();

			if (it != _clientsMap.end())
				throw ClientIsInChannelException();

		addClientLabel:
			_addClient(fdClient, 0);
		}

		int		get_size() const {return _clientsMap.size();}
		void	invite(int fdClient) {_addClient(fdClient, IRC_CHANNEL_INVITED);}
		bool	isInvited(int fdClient) {return IRC_FLAG_GET(_getClient(fdClient), IRC_CHANNEL_INVITED);}

		int		removeClient(int fdClient)
		{
			IRC_AUTO	it = _clientsMap.find(fdClient);

			if (it == _clientsMap.end())
				throw ClientNotInChannelException();
			_clientsMap.erase(it);
			return (get_size());
		}

		bool	havePerm(int fdClient)
		{
			int32_t	clientFlag = _getClient(fdClient);
			if (clientFlag == -1)
				throw ClientNotInChannelException();
			return IRC_FLAG_GET(clientFlag, IRC_CHANNEL_OPERATOR);
		}

		void	givePerm(int userClient, int targetClient)
		{
			if (havePerm(userClient))
			{
				IRC_AUTO	it = _clientsMap.find(targetClient);

				if (it == _clientsMap.end())
					throw ClientNotInChannelException();
				IRC_FLAG_SET(it->second, IRC_CHANNEL_OPERATOR);
			}
		}
		void	removePerm(int targetClient)
		{
			IRC_AUTO	it = _clientsMap.find(targetClient);

			if (it == _clientsMap.end())
				throw ClientNotInChannelException();
			IRC_FLAG_DEL(it->second, IRC_CHANNEL_OPERATOR);
		}
		void	ignoredFlag(int fd, int32_t ignored)
		{
			IRC_AUTO	it = _clientsMap.find(fd);

			if (it == _clientsMap.end())
				throw ClientNotInChannelException();
			if (ignored)
				IRC_FLAG_SET(it->second, IRC_CHANNEL_IGNORED);
			else
				IRC_FLAG_DEL(it->second, IRC_CHANNEL_IGNORED);
		}

		void	sendMsg(const str &string) const
		{
			_broadcast(string);
		}

		GETTER(int, _userLimit);
		GETTER(str, _topic);
		GETTER(time_t, _topicSetTime);
		GETTER(str, _topicSetterNickName);
		GETTER(str, _password);
		GETTER(int32_t, _flag);
		GETTER(std::map<int COMMA int32_t>, _clientsMap);

		GETTER_C(int, _userLimit);
		GETTER_C(str, _topic);
		GETTER_C(time_t, _topicSetTime);
		GETTER_C(str, _topicSetterNickName);
		GETTER_C(str, _password);
		GETTER_C(int32_t, _flag);
		GETTER_C(std::map<int COMMA int32_t>, _clientsMap);

		SETTER(int, _userLimit);
		SETTER(str, _topic);
		SETTER(time_t, _topicSetTime);
		SETTER(str, _topicSetterNickName);
		SETTER(str, _password);
		SETTER(int32_t, _flag);

		EXCEPTION(InvalidChannelNameException,	"Invalid channel name.");
		EXCEPTION(ClientNotInChannelException,	"Client not in channel.");
		EXCEPTION(ClientIsInChannelException,	"Client is already in channel.");
		EXCEPTION(InvalidChannelKeyException,	"The Channel Key is incorrect");
		EXCEPTION(InviteOnlyChannelException,	"Its a invite Only channel");
};
