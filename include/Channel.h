#ifndef CHANNEL_H
# define CHANNEL_H

# include <vector>
# include <ircRegex.h>
# include <irc.h>
# include <time.h>

# define	R_CHANNEL_PREFIX	R_CHAR_GROUP("&#+")
# define	R_CHANNEL_ID		"!" R_X_EXACT(R_CHAR_GROUP(R_DIGIT_RANGE R_UPPER_RANGE),5)
# define	R_CHANNEL_CHAR		R_CHAR_INV_GROUP(" \x07,")

# define	R_CHANNEL_NAME		R_ALTERNATION											\
								(														\
									R_CHANNEL_PREFIX	R_1_TO_Y(R_CHANNEL_CHAR, 49),	\
									R_CHANNEL_ID		R_1_TO_Y(R_CHANNEL_CHAR, 45)	\
								)

# define	R_CHANNEL_KEY_CHAR	R_CHAR_INV_GROUP(" \x09\x10\x11\x12\x13")

# define	R_CHANNEL_KEY		R_1_TO_Y(R_CHANNEL_KEY_CHAR, 23)

# define	R_CAPTURE_CHANNEL_NAME	R_CAPTURE(R_CHANNEL_NAME)
# define	R_CAPTURE_CHANNEL_KEY	R_CAPTURE(R_CHANNEL_KEY)

class Client;

bool	intInVector(std::vector<int> &v, int x)
{
	std::vector<int>::iterator	it;
	for (it = v.begin(); it != v.end(); ++it)
	{
		if (*it == x)
			return 1;
	}
	return 0;
}

bool	removeIfVector(std::vector<int> &v, int x)
{
	bool returnValue = 0;
	std::vector<int>::iterator	it;
	for (it = v.begin(); it != v.end(); ++it)
	{
		if (*it == x)
		{
			v.erase(it);
			returnValue = 1;
		}
	}
	return returnValue;
}

class Channel
{
	//TODO pour l instant tous est public =
	private:
		str						_name;
		bool					_inviteOnlyChannel;

		bool					_activePassword;
		str						_password;

		int						_userLimit;
		str						_topic;
		time_t					_topicSetTime;
		str						_topicSetterNickName;

		bool					_topicIsSet;
		bool					_topicPermNeeded;

		std::vector<int>		_fdClient;
		std::vector<int>		_fdAdminClient;

		std::vector<int>		_invitedClient;		

		void	_addClient(int fdClient, int perm);
	
	public:
		Channel(str name, int);
		~Channel(void);

		int		get_size() const {return _fdAdminClient.size() + _fdClient.size();}
		void	invite(int fdClient) {_invitedClient.push_back(fdClient);}
		bool	isInvited(int fdClient) {return intInVector(_invitedClient, fdClient);}
		void	addClient(int fdClient, const str *password);
		int		removeClient(int fdClient); //TODO remove call by server, need to check if size == 0 to delete it

		bool	havePerm(int fdClient);
		void	givePerm(int userClient, int targetClient);
		void	removePerm(int targetClient);

		void	_send(const str &str);

		GETTER(str, _name);
		GETTER(str, _topic);
		GETTER(bool, _topicPermNeeded);
		GETTER(bool, _topicIsSet);
		GETTER(time_t, _topicSetTime);
		GETTER(str, _topicSetterNickName);
		GETTER(str, _password);
		GETTER(bool, _activePassword);
		GETTER(bool, _inviteOnlyChannel);
		GETTER(std::vector<int>, _fdClient);
		GETTER(std::vector<int>, _fdAdminClient);
		GETTER(std::vector<int>, _invitedClient);

		GETTER_C(int, _userLimit);
		GETTER_C(str, _name);
		GETTER_C(str, _topic);
		GETTER_C(bool, _topicPermNeeded);
		GETTER_C(bool, _topicIsSet);
		GETTER_C(time_t, _topicSetTime);
		GETTER_C(str, _topicSetterNickName);
		GETTER_C(str, _password);
		GETTER_C(bool, _activePassword);
		GETTER_C(bool, _inviteOnlyChannel);
		GETTER_C(std::vector<int>, _fdClient);
		GETTER_C(std::vector<int>, _fdAdminClient);
		GETTER_C(std::vector<int>, _invitedClient);

		SETTER(int, _userLimit);
		SETTER(str, _topic);
		SETTER(bool, _topicPermNeeded);
		SETTER(bool, _topicIsSet);
		SETTER(time_t, _topicSetTime);
		SETTER(str, _topicSetterNickName);
		SETTER(str, _password);
		SETTER(bool, _activePassword);
		SETTER(bool, _inviteOnlyChannel);
		SETTER(std::vector<int>, _invitedClient);

		EXCEPTION(InvalidChannelNameException,	"Invalid channel name.");
		EXCEPTION(ClientNotInChannelException,	"Client not in channel.");
		EXCEPTION(InvalidChannelKeyException,	"The Channel Key is incorrect");
		EXCEPTION(InviteOnlyChannelException,	"Its a invite Only channel");
};

#endif // CHANNEL_H
