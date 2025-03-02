#pragma once

# include <Server.h>
# include <Channel.h>

void	Server::_joinAddAllChannel(std::vector<str> &vecChannel, std::vector<str> &vecKey, Client &client)
{
	size_t	j;
	if (vecKey.size() > vecChannel.size())
		goto needMoreParam;
	for (j = 0; j < vecKey.size(); j++)
		_addChannel(vecChannel[j], &vecKey[j], client);
	for (; j < vecChannel.size(); j++)
		_addChannel(vecChannel[j], NULL, client);
	return ;

needMoreParam:
	return _send(client, _architect.ERR_NEEDMOREPARAMS(client.getTargetName(), "JOIN"));
}

IRC_COMMAND_DEF(JOIN)
{
	IRC_WARN("joining %s", command.c_str());
	_seeker.feedString(command);
	_seeker.rebuild(R_MIDDLE_PARAM);
	_seeker.findall();
	std::vector<str>	argv = _seeker.get_matches();
	if (argv.size() == 0)
		return _send(client, _architect.ERR_NEEDMOREPARAMS(client.getTargetName(), "JOIN"));
	
	_seeker.feedString(argv[0]);
	_seeker.rebuild(R_CAPTURE_CHANNEL_NAME);
	_seeker.findall();
	std::vector<str>	vecChannel = _seeker.get_matches();

	if (argv.size() == 1)
	{
		IRC_WARN("no key given");
		for (size_t j = 0; j < vecChannel.size(); j++)
			_addChannel(vecChannel[j], NULL, client);
		return ;
	}
	if (argv.size() == 2)
	{
		IRC_WARN("parsing  keys");
		_seeker.feedString(argv[1]);
		_seeker.rebuild(R_CAPTURE_CHANNEL_KEY);
		_seeker.findall();
		std::vector<str>	vecPassword = _seeker.get_matches();
		_joinAddAllChannel(vecChannel, vecPassword, client);
		return ;
	}
	IRC_WARN("error too many args");
}

void	Server::_addChannel(const str &channelName, const str *channelKey, Client &client)
{
	IRC_OK("joinning channel: %s with key :%s", channelName.c_str(), channelKey ? channelKey->c_str() : "NULL");
	int32_t			fd = client.get_fd();
	Channel			*c = _getChannelByName(channelName);

	IRC_LOG("_getChannel result => %p", c);
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
	IRC_LOG("channel already exist; joining it");
	if (c->get_inviteOnlyChannel() && !c->isInvited(fd))
		goto inviteOnlyChannel;
	try
	{
		if (c->get_size() == c->get_userLimit())
			goto channelIsFull;
		c->addClient(fd, channelKey);
		goto joinChannel;
	}
	catch (std::exception &e)
	{
		if (c->get_activePassword() && (!channelKey || *channelKey != c->get_password()))
			goto badChannelKey;
		return ; //Already in channel
	}
channelDoesntExist:
	c = new Channel(channelName, fd);
	_channelMap[channelName] = c;
	goto joinChannel;
}

void	Server::_sendJoin(Client &client, Channel *channel)
{
	str	clientName = client.get_nickname();
	str channelName = channel->get_name();
	str topic = channel->get_topic();

	std::vector<str>	clientList;
	std::vector<int>	fdList;

	fdList = channel->get_fdClient();
	for (IRC_AUTO it = fdList.begin(); it != fdList.end(); ++it)
	{
		int fdClient = *it;
		IRC_AUTO s = _clients.find(fdClient);
		if (s != _clients.end())
			clientList.push_back(s->second.get_nickname());
	}

	fdList = channel->get_fdAdminClient();
	for (IRC_AUTO it = fdList.begin(); it != fdList.end(); ++it)
	{
		int fdClient = *it;
		IRC_AUTO s = _clients.find(fdClient);
		if (s != _clients.end())
			clientList.push_back("@" + s->second.get_nickname());
	} 

	str	clientListString;
	for (size_t i = 0; i < clientList.size(); i++)
	{
		if (i != 0)
			clientListString += " ";
		clientListString += clientList[i];
	}

	channel->_broadcast(_architect.CMD_JOIN(clientName, channelName.c_str()));
	_send(client, _architect.RPL_TOPIC(clientName.c_str(), channelName.c_str(), topic.c_str()));
	_send(client, _architect.RPL_NAMREPLY(clientName.c_str(), "=", channelName.c_str(), clientListString.c_str()));
	_send(client, _architect.RPL_ENDOFNAMES(clientName.c_str(), channelName.c_str()));
}
