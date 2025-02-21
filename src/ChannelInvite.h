#ifndef CHANNELTOPIC_H
# define CHANNELTOPIC_H

# include <Server.h>
# include <Client.h>
# include <Channel.h>
# include <poll.h>

void	Server::_invite(const str command, Client *client)
{
	UNUSED(command);
	//<nickname> <channel>
	
	str	nickName;
	str	channelName;

	Channel *channel = _getChannelByName(channelName);
	Client	*target = _getClientByName(nickName);
	bool			perm = 0;
	//TODO parsing

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
	catch (std::exception &e)
	{
		goto succesfullInviting;
	}

succesfullInviting:
	_send(client, _architect.RPL_INVITING(3, client->get_nickname().c_str(), target->get_nickname().c_str(), channelName.c_str()));
	channel->invite(target->get_pfd()->fd);
	_send(target, client->get_nickname() + " invited you to channel " + channelName);
	return ;

errorUserOnChannel:
	_send(client, _architect.ERR_USERONCHANNEL(4, client->get_nickname().c_str(), target->get_nickname().c_str(), channelName.c_str(), "is already on channel"));
	return ;

errorNoPerm:
	return _send(client, _architect.ERR_CHANOPRIVSNEEDED(3, client->get_nickname().c_str(), channelName.c_str(), "You're not channel operator"));
	return ;

errorYouAreNotOnChannel:
	return _send(client, _architect.ERR_NOTONCHANNEL(3, client->get_nickname().c_str(), channelName.c_str(), "You're not on that channel"));
	return ;

errorNoSuchNick:
	_send(client, _architect.ERR_NOSUCHNICK(3, client->get_nickname().c_str(), nickName.c_str(), "No such nick/channel"));
	return ;

errorNoSuchChannel:
	_send(client, _architect.ERR_NOSUCHCHANNEL(3, client->get_nickname().c_str(), channelName.c_str(), "No such channel"));
	return ;

}


#endif
