#pragma once

# include <Server.h>
# include <Channel.h>
#include <vector>

void	Server::_userJoinChannel(const str &channelName, const str *channelKey, Client &client)
{
	IRC_LOG(IRC_ANALYST "joinning channel: %s with key %s", channelName.c_str(), channelKey ? channelKey->c_str() : "NULL");
	int32_t			fd = client.get_fd();
	Channel			*c = _getChannelByName(channelName);

	if (c)
		goto channelExist;
	goto channelDoesntExist;

joinChannel:
	client.joinChannel(c);
	return _sendJoin(client, c);
inviteOnlyChannel:
	return _send(client, _architect.ERR_INVITEONLYCHAN(client.getTargetName(), channelName.c_str()));
channelIsFull:
	return _send(client, _architect.ERR_CHANNELISFULL(client.getTargetName(), channelName.c_str()));
badChannelKey:
	return _send(client, _architect.ERR_BADCHANNELKEY(client.getTargetName(), channelName.c_str()));

channelExist:
	if (IRC_FLAG_GET(c->get_flag(), IRC_CHANNEL_INVITE_ONLY) && !c->isInvited(fd))
		goto inviteOnlyChannel;
	try
	{
		if (c->get_size() >= c->get_userLimit())
			goto channelIsFull;
		c->addClient(fd, channelKey);
		goto joinChannel;
	}
	catch (std::exception &e)
	{
		if (IRC_FLAG_GET(c->get_flag(), IRC_CHANNEL_ACTIVE_PASSWORD) && (!channelKey || *channelKey != c->get_password()))
			goto badChannelKey;
		return ;
	}

channelDoesntExist:
	c = new Channel(channelName, fd);
	_channelMap[channelName] = c;
	goto joinChannel;
}

void	Server::_sendJoin(Client &client, Channel *channel)
{
	str	clientName = client.get_nickname();
	str channelName = channel->getTargetName();
	str topic = channel->get_topic();

	str					clientListString;
	int					addSpace = 0;

	IRC_AUTO fdList = channel->get_clientsMap();
	for (IRC_AUTO it = fdList.begin(); it != fdList.end(); ++it)
	{
		IRC_AUTO s = _clients.find(it->first);
		if (s != _clients.end())
		{
			str name = IRC_FLAG_GET(it->second, IRC_CHANNEL_OPERATOR) ? str("@") : str();
			name += s->second.get_nickname();
			if (addSpace++)
				clientListString += " ";
			clientListString += name;
		}
	}

	channel->sendMsg(_architect.CMD_JOIN(clientName, channelName.c_str()));
	_send(client, _architect.RPL_TOPIC(clientName.c_str(), channelName.c_str(), topic.c_str()));
	_send(client, _architect.RPL_NAMREPLY(clientName.c_str(), "=", channelName.c_str(), clientListString.c_str()));
	_send(client, _architect.RPL_ENDOFNAMES(clientName.c_str(), channelName.c_str()));
}

IRC_COMMAND_DEF(JOIN)
{
	const std::vector<str> &param = _parsingParam(command);
	if (param.size() == 0)
		return _send(client, _architect.ERR_NEEDMOREPARAMS(client.getTargetName(), "JOIN"));
	
	_seeker.feedString(param[0]);
	_seeker.rebuild(R_CAPTURE_CHANNEL_NAME);
	_seeker.findall();
	std::vector<str>	vecChannel = _seeker.get_matches();

	if (param.size() == 1)
	{
		if (param[0] == "0")
			return _partAllChannel(client, 0);
		for (size_t j = 0; j < vecChannel.size(); j++)
			_userJoinChannel(vecChannel[j], NULL, client);
		return ;
	}
	if (param.size() == 2)
	{
		_seeker.feedString(param[1]);
		_seeker.rebuild(R_CAPTURE_CHANNEL_KEY);
		_seeker.findall();
		std::vector<str>	vecKey = _seeker.get_matches();
		size_t	j;
		if (vecKey.size() > vecChannel.size())
			goto needMoreParam;
		for (j = 0; j < vecKey.size(); j++)
			_userJoinChannel(vecChannel[j], &vecKey[j], client);
		for (; j < vecChannel.size(); j++)
			_userJoinChannel(vecChannel[j], NULL, client);
		return ;

	needMoreParam:
		return _send(client, _architect.ERR_NEEDMOREPARAMS(client.getTargetName(), "JOIN"));
	}
}
