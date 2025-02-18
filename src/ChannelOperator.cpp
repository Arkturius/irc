#include <cerrno>
#include <climits>
#include <cstddef>
#include <cstdlib>
#include <exception>
#include <poll.h>
#include <string>
#include <vector>

#include <Server.h>
#include <Client.h>
#include <Channel.h>

Client	*Server::_getClientByName(const str userName)
{
	for (IRC_AUTO it = _clients.begin(); it != _clients.end(); ++it)
	{
		if (it->second.get_nickname() == userName)
			return &it->second;
	}
	return NULL;
}

Channel	*Server::_getChannelByName(const str Name)
{
	IRC_AUTO it = _channelMap.find(Name);
	if (it != _channelMap.end())
		return it->second;
	return NULL;
}

void	Server::_join(const str cmd, Client *client)
{
	str	command = cmd.substr(5, cmd.size());
	std::vector<str>	vecChannel;
	std::vector<str>	vecPassword;
	uint				vecKeyLen = 0;
	uint				j = 0;

	IRCSeeker	rparserChannel(R_CAPTURE_CHANNEL_NAME);
	IRCSeeker	rparserKey(R_CAPTURE_CHANNEL_KEY);

	rparserChannel.findall(command);
	vecChannel = rparserChannel.get_matches();
	
	size_t spaceIndex = command.find(" ");
	if (spaceIndex != std::string::npos)
	{
		command = command.substr(spaceIndex, cmd.size());
		rparserKey.findall(command);
		vecPassword = rparserKey.get_matches();
		vecKeyLen = vecPassword.size();
	}

	if (vecKeyLen > vecChannel.size())
		; //TODO ERR_NEEDMOREPARAMS ?
	for (; j < vecKeyLen; j++)
	{
		str	a = vecPassword[j];
		_addChannel(vecChannel[j], &a, client);
	}
	for (; j < vecChannel.size(); j++)
	{
		_addChannel(vecChannel[j], NULL, client);
	}
}

void	Server::_kick(const str command, Client *client)
{
	 //Parameters: <channel> *( "," <channel> ) <user> *( "," <user> ) [<comment>]
	
	std::vector<str>	vecChannel;
	std::vector<str>	vecUser;
	str					*comment = NULL;
	Client				*target;
	//TODO parsing
	
	if (vecChannel.size() == 1)
	{
		_kickChannel(vecChannel[0], client, vecUser[0], comment);
	}
	else if (vecChannel.size() == vecUser.size())
	{
		for (size_t i = 0; i < vecUser.size(); i++)
		{
			_kickChannel(vecChannel[i], client, vecUser[i], comment);
		}
	}
	else
		; //TODO erreur param
	

	(void)command;(void)client;
}
void	Server::_topic(const str command, Client *client)
{
	//TOPIC channel => print (can be not set (erreur))
	//TOPIC channel : => clear 
	//TOPIC channel : topicContent => rempli

	//TODO parsing
	(void)command;(void)client;
	str		channelName;
	Channel	*channel = _getChannelByName(channelName);
	int		perm = 0;
	if (!channel)
		return _send(client, _architect.ERR_NOSUCHCHANNEL("", 3, client->get_nickname().c_str(), channelName.c_str(), "No such channel"));
	try { perm = channel->havePerm(client->get_pfd()->fd); }
	catch (std::exception &e)
	{
		return _send(client, _architect.ERR_NOTONCHANNEL("", 3, client->get_nickname().c_str(), channelName.c_str(), "You're not on that channel"));
	}
	bool	newTopic = 1;
	if (newTopic)
	{
		str	topic;
		
		if (!perm && channel->get_topicPermNeeded())
			return _send(client, _architect.ERR_CHANOPRIVSNEEDED("", 3, client->get_nickname().c_str(), channelName.c_str(), "You're not channel operator"));
		else
		{
			channel->set_topic(topic);
			channel->set_topicIsSet(1);
			channel->set_topicSetterNickName(client->get_nickname());
			channel->set_topicSetTime(time(NULL));
			_sendTopic(client, channel);
			channel->_send(_architect.RPL_TOPIC("", 3, client->get_nickname().c_str(), channelName.c_str(), topic.c_str()));
		}
	}
	else
	{
		if (channel->get_topicIsSet())
		{
			str	topic = channel->get_topic();
			_sendTopic(client, channel);
		}
		else
			return _send(client, _architect.RPL_NOTOPIC("", 3, client->get_nickname().c_str(), channelName.c_str(), "No topic is set"));
	}
}


bool	Server::_individualMode(bool plus, char mode, const str &modeArguments, Channel *target, Client *client)
{
	switch (mode)
	{
		case ('o'):
		{
			Client	*userTarget = NULL;
			try
			{
				if (!userTarget)
					throw UnexpectedErrorException();
				if (plus)
					target->givePerm(client->get_pfd()->fd, userTarget->get_pfd()->fd);
				else
					target->removePerm(userTarget->get_pfd()->fd);
				return 0;
			}
			catch (std::exception &e)
			{
				return _send(client, _architect.ERR_USERNOTINCHANNEL("", 4, client->get_nickname().c_str(), userTarget->get_nickname().c_str(), target->get_name().c_str(), "They aren't on that channel")), 1;
			}
		}
		case ('i'):
			return target->set_inviteOnlyChannel(plus), 0;
		case ('t'):
			return target->set_topicPermNeeded(plus), 0;
		case ('k'):
		{
			if (!plus && target->get_password() == modeArguments)
				return target->set_activePassword(0), 0;
			else if (plus)
			{
				//regex mot de pass => INVALIDEMODEPARAM
				target->set_password(modeArguments);
				target->set_activePassword(1);
				return 0;
			}
			else
				return _send(client, _architect.ERR_INVALIDMODEPARAM("", 5, client->get_nickname().c_str(), target->get_name().c_str(), "k", modeArguments.c_str(), "To disable the password you need to enter the correct password")), 1;
		}
		case ('l'):
		{
			long	ele;
			char	*tmp;

			ele = strtol(modeArguments.c_str(), &tmp, 10);
			if (errno == ERANGE || ele < 0 || ele > INT_MAX || *tmp)
				return _send(client, _architect.ERR_INVALIDMODEPARAM("", 5, client->get_nickname().c_str(), target->get_name().c_str(), "l", modeArguments.c_str(), "The limite sould be a possitiv integer")), 1;
			if (plus)
				target->set_userLimit(ele);
			else
				target->set_userLimit(INT_MAX);
			return 0;
		}
	}
	return _send(client, _architect.ERR_UNKNOWNMODE("", 3, client->get_nickname().c_str(), mode, "is unknown mode char to me")), 1;
}

void	Server::_mode(const str command, Client *client)
{
	//<target> [<modestring> [<mode arguments>...]]
	(void)command;(void)client;
	str		targetName;
	str		modeString;
	Channel	*target;
	bool	giveModeString = 0;
	//TODO parsing

	target = _getChannelByName(targetName);
	if (!target)
		return _send(client, _architect.ERR_NOSUCHNICK("", 3, client->get_nickname().c_str(), targetName.c_str(), "No such nick/channel"));
	if (!giveModeString)
		goto getModeOfChannel;
	try {
		if (target->havePerm(client->get_pfd()->fd) == 0)
			return _send(client, _architect.ERR_CHANOPRIVSNEEDED("", 3, client->get_nickname().c_str(), targetName.c_str(), "You're not channel operator"));
	}
	catch (std::exception &e)
	{
		return _send(client, _architect.ERR_NOTONCHANNEL("", 3, client->get_nickname().c_str(), targetName.c_str(), "You're not on that channel"));
	}
	// while (return == 0)
	// _individualMode(bool plus, char mode, const str &modeArguments, Channel *target, Client *client)

getModeOfChannel:
	_sendModeIs(client, target);
}
void	Server::_invite(const str command, Client *client)
{
	//<nickname> <channel>
	
	str	nickName;
	str	channelName;

	Channel *channel = _getChannelByName(channelName);
	Client	*target = _getClientByName(nickName);
	//TODO parsing

	if (!channel)
		return _send(client, _architect.ERR_NOSUCHCHANNEL("", 3, client->get_nickname().c_str(), channelName.c_str(), "No such channel"));
	if (!target)
		return _send(client, _architect.ERR_NOSUCHNICK("", 3, client->get_nickname().c_str(), nickName.c_str(), "No such nick/channel"));
	bool			perm = 0;
	try { perm = channel->havePerm(client->get_pfd()->fd); }
	catch (std::exception &e)
	{
		return _send(client, _architect.ERR_NOTONCHANNEL("", 3, client->get_nickname().c_str(), channelName.c_str(), "You're not on that channel"));
	}
	if (!perm && channel->get_inviteOnlyChannel())
		return _send(client, _architect.ERR_CHANOPRIVSNEEDED("", 3, client->get_nickname().c_str(), channelName.c_str(), "You're not channel operator"));
	try
	{
		channel->havePerm(target->get_pfd()->fd);
		return _send(client, _architect.ERR_USERONCHANNEL("", 4, client->get_nickname().c_str(), target->get_nickname().c_str(), channelName.c_str(), "is already on channel"));
	}
	catch (std::exception &e)
	{
		//RPL_INVITING
		//+ envoie msg d invitation a target
	}
	(void)command;
}

void	Server::_addChannel(str channelName, str *channelKey, Client *client)
{
	struct pollfd	*pfd = client->get_pfd();
	Channel			*c = _getChannelByName(channelName);

	if (c)
	{
		if (c->get_inviteOnlyChannel())
			return _send(client, _architect.ERR_INVITEONLYCHAN("", 3, client->get_nickname().c_str(), channelName.c_str(), "Cannot join channel (+i)"));
		try
		{
			if (c->get_size() == c->get_userLimit())
				return _send(client, _architect.ERR_CHANNELISFULL("", 3, client->get_nickname().c_str(), channelName.c_str(), "Cannot join channel (+l)"));
			c->addClient(pfd->fd, channelKey);
		}
		catch (std::exception &e)
		{
			return _send(client, _architect.ERR_BADCHANNELKEY("", 3, client->get_nickname().c_str(), channelName.c_str(), "Cannot join channel (+k)"));
		}
	}
	else
	{
		Channel	channel(channelName, pfd->fd);
		c = &channel;
		_channelMap[channelName] = c;
		c->addClient(pfd->fd, channelKey);
	}
	client->joinChannel(c);
	_sendJoin(client, c);
}

void	Server::_removeChannel(str channelName, Client *client)
{
	struct pollfd	*pfd = client->get_pfd();
	Channel			*c = NULL;
	IRC_AUTO		s = _channelMap.find(channelName);

	if (s != _channelMap.end())
	{
		c = s->second;
		int size = c->removeClient(pfd->fd);
		if (size == 0)
			_channelMap.erase(s);
	}
	client->leaveChannel(c);
}

void	Server::_kickChannel(str channelName, Client *admin, str kickedName, str *comment)
{
	Client	*kicked = _getClientByName(kickedName);
	IRC_AUTO		s = _channelMap.find(channelName);
	struct pollfd	*pfdAdmin;
	struct pollfd	*pfdKicked;
	Channel			*c = NULL;
	int				size;

	if (!kicked)
		goto targetDontExist;
	pfdAdmin = admin->get_pfd();
	pfdKicked = kicked->get_pfd();

	if (s != _channelMap.end())
	{
		c = s->second;
		try {
			if (!c->havePerm(pfdAdmin->fd))
				goto clientDontHaveThePerm;
		}
		catch (std::exception &e) {
			goto adminNotInChannel;}
		try {
			size = c->removeClient(pfdKicked->fd);}
		catch (std::exception &e) {
			goto targetDontExist;}
		if (size == 0)
			_channelMap.erase(s);
		if (comment)
			kicked->leaveChannel(c, *comment);
		else
			kicked->leaveChannel(c);
	}
	else
		return _send(admin, _architect.ERR_NOSUCHCHANNEL("", 3, admin->get_nickname().c_str(), channelName.c_str(), "No such channel"));
clientDontHaveThePerm:
	return _send(admin, _architect.ERR_CHANOPRIVSNEEDED("", 3, admin->get_nickname().c_str(), channelName.c_str(), "You're not channel operator"));
targetDontExist:
	return _send(admin, _architect.ERR_USERNOTINCHANNEL("", 4, admin->get_nickname().c_str(), kickedName.c_str(),channelName.c_str(), "They aren't on that channel"));
adminNotInChannel:
	return _send(admin, _architect.ERR_NOTONCHANNEL("", 3, admin->get_nickname().c_str(), channelName.c_str(), "You're not on that channel"));
}
