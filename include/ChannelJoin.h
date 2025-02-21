#ifndef CHANNEL_JOIN_H
# define CHANNEL_JOIN_H

# include <Server.h>
# include <Channel.h>
# include <Client.h>
# include <poll.h>

IRC_COMMAND_DEF(JOIN)
{
	UNUSED(command);
	UNUSED(client);
}

void	Server::_joinAddAllChannel
(std::vector<str> &vecChannel, std::vector<str> &vecKey, Client *client)
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
	_send(client, _architect.ERR_NEEDMOREPARAMS(client->get_nickname().c_str(), "JOIN", "Not enough parameters"));
}

void	Server::_join(const str &command, Client *client)
{
	_seeker.feedString(command);
	_seeker.rebuild(R_MIDDLE_PARAM);
	_seeker.findall();
	std::vector<str>	&argv = _seeker.get_matches();
	if (argv.size() == 0)
		return _send(client, _architect.ERR_NEEDMOREPARAMS(client->get_nickname().c_str(), "JOIN", "Not enough parameters"));
	
	_seeker.feedString(argv[0]);
	_seeker.rebuild(R_CAPTURE_CHANNEL_NAME);
	_seeker.findall();
	std::vector<str>	&vecChannel = _seeker.get_matches();

	if (argv.size() == 1)
	{
		for (size_t j = 0; j < vecChannel.size(); j++)
			_addChannel(vecChannel[j], NULL, client);
		return ;
	}
	if (argv.size() == 2)
	{
		_seeker.feedString(argv[1]);
		_seeker.rebuild(R_CAPTURE_CHANNEL_KEY);
		_seeker.findall();
		std::vector<str>	&vecPassword = _seeker.get_matches();
		_joinAddAllChannel(vecChannel, vecPassword, client);
		return ;
	}
}

void	Server::_addChannel(const str &channelName, const str *channelKey, Client *client)
{
	struct pollfd	*pfd = client->get_pfd();
	Channel			*c = _getChannelByName(channelName);

	if (c)
		goto channelExist;
	goto channelDoesntExist;

joinChannel:
	c->addClient(pfd->fd, channelKey);
	client->joinChannel(c);
	return _sendJoin(client, c);
inviteOnlyChannel:
	return _send(client, _architect.ERR_INVITEONLYCHAN(client->get_nickname().c_str(), channelName.c_str(), "Cannot join channel (+i)"));
channelIsFull:
	return _send(client, _architect.ERR_CHANNELISFULL(client->get_nickname().c_str(), channelName.c_str(), "Cannot join channel (+l)"));
badChannelKey:
	return _send(client, _architect.ERR_BADCHANNELKEY(client->get_nickname().c_str(), channelName.c_str(), "Cannot join channel (+k)"));

channelExist:
	if (c->get_inviteOnlyChannel() && !c->isInvited(pfd->fd))
		goto inviteOnlyChannel;
	try
	{
		if (c->get_size() == c->get_userLimit())
			goto channelIsFull;
		goto joinChannel;
	}
	catch (std::exception &e)
	{ goto badChannelKey; }
channelDoesntExist:
	Channel	channel(channelName, pfd->fd);
	c = &channel;
	_channelMap[channelName] = &channel;
	goto joinChannel;
}

#endif
