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
	_send(client, _architect.RPL_INVITING(client->get_nickname().c_str(), target->get_nickname().c_str(), channelName.c_str()));
	channel->invite(target->get_pfd()->fd);
	_send(target, client->get_nickname() + " invited you to channel " + channelName);
	return ;

errorUserOnChannel:
	_send(client, _architect.ERR_USERONCHANNEL(client->get_nickname().c_str(), target->get_nickname().c_str(), channelName.c_str()));
	return ;

errorNoPerm:
	return _send(client, _architect.ERR_CHANOPRIVSNEEDED(client->get_nickname().c_str(), channelName.c_str()));
	return ;

errorYouAreNotOnChannel:
	return _send(client, _architect.ERR_NOTONCHANNEL(client->get_nickname().c_str(), channelName.c_str()));
	return ;

errorNoSuchNick:
	_send(client, _architect.ERR_NOSUCHNICK(client->get_nickname().c_str(), nickName.c_str()));
	return ;

errorNoSuchChannel:
	_send(client, _architect.ERR_NOSUCHCHANNEL(client->get_nickname().c_str(), channelName.c_str()));
	return ;

}


#endif
