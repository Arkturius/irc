#ifndef CHANNEL_JOIN_H
# define CHANNEL_JOIN_H

# include <Server.h>
# include <Channel.h>
# include <Client.h>
# include <poll.h>

void	Server::_joinAddAllChannel
(std::vector<str> vecChannel, std::vector<str> vecKey, Client *client)
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
	_send(client, _architect.ERR_NEEDMOREPARAMS(3, client->get_nickname().c_str(), "JOIN", "Not enough parameters"));
}

void	Server::_join(const str &command, Client *client)
{
	//TODO parsing
	std::vector<str>	vecChannel;
	std::vector<str>	vecPassword;
	UNUSED(command);

	_joinAddAllChannel(vecChannel, vecPassword, client);
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
	return _send(client, _architect.ERR_INVITEONLYCHAN(3, client->get_nickname().c_str(), channelName.c_str(), "Cannot join channel (+i)"));
channelIsFull:
	return _send(client, _architect.ERR_CHANNELISFULL(3, client->get_nickname().c_str(), channelName.c_str(), "Cannot join channel (+l)"));
badChannelKey:
	return _send(client, _architect.ERR_BADCHANNELKEY(3, client->get_nickname().c_str(), channelName.c_str(), "Cannot join channel (+k)"));

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
