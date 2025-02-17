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
	//parsing
	
	if (vecChannel.size() == 1)
	{
		target = _getClientByName(vecUser[0]);
		_kickChannel(vecChannel[0], client, target, comment);
	}
	else if (vecChannel.size() == vecUser.size())
	{
		for (size_t i = 0; i < vecUser.size(); i++)
		{
			target = _getClientByName(vecUser[i]);
			_kickChannel(vecChannel[i], client, target, comment);
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

	//do all the parsing
	(void)command;(void)client;
	str		channelName;
	Channel	*channel = _getChannelByName(channelName);
	int		perm = 0;
	if (!channel)
	{
		//ERR_NOSUCHCHANNEL 
	}
	try
	{
		perm = channel->havePerm(client->get_pfd()->fd);
	}
	catch (std::exception &e)
	{
		//ERR_NOTONCHANNEL
	}
	bool	newTopic = 1;
	if (newTopic)
	{
		str	topic;
		
		if (!perm && channel->get_topicPermNeeded())
			;	//ERR_CHANOPRIVSNEEDED 
		else
		{
			channel->set_topic(topic);
			channel->set_topicIsSet(1);
			//RPL_TOPIC 
			//RPL_TOPICWHOTIME
			// + envoyer a TOUS(meme source) LES MEMBRE DU CHANNEL UNE UPDATE (uniquement <topic>)
		}
	}
	else
	{
		if (channel->get_topicIsSet())
		{
			str	topic = channel->get_topic();
			//RPL_TOPIC 
			//RPL_TOPICWHOTIME
		}
		else
		{
			//RPL_NOTOPIC
		}

	}
}


void	Server::_individualMode(bool plus, char mode, const str &modeArguments, Channel *target, Client *client)
{
	Client	*userTarget;
	if (mode == 'o')
	{
		try
		{
			if (plus)
				target->givePerm(client->get_pfd()->fd, userTarget->get_pfd()->fd);
			else
				target->removePerm(client->get_pfd()->fd, userTarget->get_pfd()->fd);
		}
		catch (std::exception())
		{
			//ERR_USERNOTINCHANNEL
		}
	}
	if (mode == 'i')
		target->set_inviteOnlyChannel(plus);
	if (mode == 't')
		target->set_topicPermNeeded(plus);
	if (mode == 'k')
	{
		if (!plus && target->get_password() == modeArguments)
			target->set_activePassword(0);
		else if (!plus)
		{
			//TODO regex pour verif que modeArguments est valide => ERR_INVALIDMODEPARAM
			target->set_password(modeArguments);
			target->set_activePassword(1);
		}
		else
			;	//ERR_INVALIDMODEPARAM
	}
	if (mode == 'l')
	{
		//regex nombre xd
		if (plus)
			target->set_userLimit(std::atoi(modeArguments.c_str()));
		else
			target->set_userLimit(INT_MAX);
	}
}

void	Server::_mode(const str command, Client *client)
{
	//<target> [<modestring> [<mode arguments>...]]
	(void)command;(void)client;
	str		targetName;
	str		modeString;
	Channel	*target;
	bool	giveModeString = 0;


	target = _getChannelByName(targetName);
	if (!target)
		;//ERR_NOSUCHNICK
	if (!giveModeString)
		goto getModeOfChannel;
	try {
		if (target->havePerm(client->get_pfd()->fd) == 0)
			; // ERR_CHANOPRIVSNEEDED
	}
	catch (std::exception &e)
	{
		// ERR_NOTINCHANNEL
	}
	// _individualMode(bool plus, char mode, const str &modeArguments, Channel *target, Client *client)

getModeOfChannel:
	; // RPL_CHANNELMODEIS
}
void	Server::_invite(const str command, Client *client)
{
	//<nickname> <channel>
	
	str	nickName;
	str	channelName;

	Channel *channel = _getChannelByName(channelName);
	Client	*target = _getClientByName(nickName);

	if (!channel)
		;//ERR_NOSUCHCHANNEL
	if (!target)
		; // ERR_NOSUCHNICK
	bool			perm = 0;
	try
	{
		perm = channel->havePerm(client->get_pfd()->fd);
	}
	catch (std::exception &e)
	{
		//ERR_NOTONCHANNEL
	}
	if (!perm && channel->get_inviteOnlyChannel())
		;//ERR_CHANOPRIVSNEEDED
	
	try
	{
		channel->havePerm(target->get_pfd()->fd);
		// ERR_USERONCHANNEL 
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
			; //ERR_INVITEONLYCHAN
		try
		{
			if (c->get_size() == c->get_userLimit())
				; //ERR_CHANNELISFULL 
			c->addClient(pfd->fd, channelKey);
		}
		catch (std::exception &e)
		{
			//ERR_BADCHANNELKEY 
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
	_send_join(client, c);
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

void	Server::_kickChannel(str channelName, Client *admin, Client *kicked, str *comment)
{
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
	{
		//ERR_NOSUCHCHANNEL
	}
clientDontHaveThePerm:
	; //ERR_CHANOPRIVSNEEDED
targetDontExist:
	; //ERR_USERNOTINCHANNEL
adminNotInChannel:
	; // ERR_NOTONCHANNEL
}
