#include <cstddef>
#include <poll.h>
#include <string>
#include <vector>

#include <Server.h>
#include <Client.h>
#include <Channel.h>
#include <RParser.h>

void	Server::_join(const str cmd, Client *client)
{
	str	command = cmd.substr(5, cmd.size());
	std::vector<str>	vecChannel;
	std::vector<str>	vecPassword;
	uint				vecKeyLen = 0;
	uint				j = 0;

	RParser	rparserChannel(R_CAPTURE_CHANNEL_NAME);
	RParser	rparserKey(R_CAPTURE_CHANNEL_KEY);

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
		; //TODO prb
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
	//TODO soit 1channel et X user ; soit Nchannel et N user 
	//_kickChannel(channelName, client, clientTmp, comment)
	(void)command;(void)client;
}

void	Server::_topic(const str command, Client *client)
{
	//TOPIC chennel => print (can be not set (erreur))
	//TOPIC channel : => clear 
	//TOPIC channel : topicContent => rempli
	(void)command;(void)client;
}
void	Server::_mode(const str command, Client *client)
{
	(void)command;(void)client;
}
void	Server::_invite(const str command, Client *client)
{
	(void)command;(void)client;
}

void	Server::_addChannel(str channelName, str *channelKey, Client *client)
{
	struct pollfd	*pfd = client->get_pfd();
	Channel			*c = NULL;
	IRC_AUTO		s = _channelMap.find(channelName);

	if (s != _channelMap.end())
	{
		c = s->second;
		c->addClient(pfd->fd, channelKey);
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
	struct pollfd	*pfdAdmin = admin->get_pfd();
	struct pollfd	*pfdKicked = kicked->get_pfd();
	Channel			*c = NULL;
	IRC_AUTO		s = _channelMap.find(channelName);
	int				size;

	if (s != _channelMap.end())
	{
		c = s->second;
		if (!c->havePerm(pfdAdmin->fd))
			goto clientDontHaveThePerm;
		size = c->removeClient(pfdKicked->fd);
		if (size == 0)
			_channelMap.erase(s);
		if (comment)
			kicked->leaveChannel(c, *comment);
		else
			kicked->leaveChannel(c);
	}
	else
	{
		//user not in channel
	}
clientDontHaveThePerm:
	; //TODO "admin" est pas admin
}
