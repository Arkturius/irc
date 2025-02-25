#ifndef CHANNELTOPIC_H
# define CHANNELTOPIC_H

# include <Server.h>
# include <Client.h>
# include <Channel.h>
# include <poll.h>
#include <vector>

void	Server::_invite(const str command, Client *client)
{
	std::vector<str>	argv;

	_seeker.feedString(command);
	_seeker.rebuild(R_MIDDLE_PARAM);
	_seeker.consumeMany();
	argv = _seeker.get_matches();
	if (argv.size() != 2)
		return _send(client, _architect.ERR_NEEDMOREPARAMS(client->getTargetName(), "INVITE"));

	str	&nickName = argv[0];
	str	&channelName = argv[1];

	Channel *channel = _getChannelByName(channelName);
	Client	*target = _getClientByName(nickName);
	bool			perm = 0;

	if (!channel)
		goto errorNoSuchChannel;
	if (!target)
		goto errorNoSuchNick;
	try { perm = channel->havePerm(client->get_pfd()->fd); }
	catch (std::exception &e)
	{
		goto errorYouAreNotOnChannel;
	}
	if (!perm && channel->get_inviteOnlyChannel())
		goto errorNoPerm;
	try
	{
		channel->havePerm(target->get_pfd()->fd);
		goto errorUserOnChannel;
	}
	IRC_CATCH

	_send(client, _architect.RPL_INVITING(client->getTargetName(), target->getTargetName(), channelName.c_str()));
	channel->invite(target->get_pfd()->fd);
	return _send(target, client->get_nickname() + " invited you to channel " + channelName);

errorUserOnChannel:
	return _send(client, _architect.ERR_USERONCHANNEL(client->getTargetName(), target->getTargetName(), channelName.c_str()));

errorNoPerm:
	return _send(client, _architect.ERR_CHANOPRIVSNEEDED(client->getTargetName(), channelName.c_str()));

errorYouAreNotOnChannel:
	return _send(client, _architect.ERR_NOTONCHANNEL(client->getTargetName(), channelName.c_str()));

errorNoSuchNick:
	return _send(client, _architect.ERR_NOSUCHNICK(client->getTargetName(), nickName.c_str()));

errorNoSuchChannel:
	return _send(client, _architect.ERR_NOSUCHCHANNEL(client->getTargetName(), channelName.c_str()));
}


#endif
