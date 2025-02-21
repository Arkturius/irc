#ifndef CHANNEL_KICK_H
# define CHANNEL_KICK_H

# include <Server.h>
# include <Channel.h>
# include <Client.h>
# include <poll.h>


void	Server::_kickAllChannel(std::vector<str> vecChannel, std::vector<str> vecUser, str *comment, Client *client)
{
	bool	j = 0;
	bool	i = 0;
	size_t	k = 0;
	size_t	m = 0;

	if (vecChannel.size() == 1)
		j = 1;
	else if (vecChannel.size() == vecUser.size())
		i = 1;
	else
		goto invalidNumberOfParams;

	for (; k < vecChannel.size() && m < vecUser.size(); k += i, m += j + i)
	{
		_kickChannel(vecChannel[k], client, vecUser[m], comment);
	}
	return ;

invalidNumberOfParams:
		_send(client, _architect.ERR_NEEDMOREPARAMS(client->get_nickname().c_str(), "KICK"));
}

void	Server::_kick(const str command, Client *client)
{
	 //Parameters: <channel> *( "," <channel> ) <user> *( "," <user> ) [<comment>]
	std::vector<str>	vecChannel;
	std::vector<str>	vecUser;
	str					*comment = NULL;
	//TODO parsing
	
	UNUSED(command);

	_kickAllChannel(vecChannel, vecUser, comment, client);
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
	if (s == _channelMap.end())
		goto noSuchChannel;

	pfdAdmin = admin->get_pfd();
	pfdKicked = kicked->get_pfd();
	c = s->second;
	try
	{
		if (!c->havePerm(pfdAdmin->fd))
			goto clientDontHaveThePerm;
	}
	catch (std::exception &e)
	{
		goto adminNotInChannel;
	}
	try
	{
		size = c->removeClient(pfdKicked->fd);
		goto succesfullKick;
	}
	catch (std::exception &e)
	{
		goto targetDontExist;
	}

succesfullKick:
	c->_broadcast(_architect.CMD_KICK(admin->get_nickname().c_str(), 1, c->get_name().c_str()));
	if (size == 0)
		_channelMap.erase(s);
	if (comment)
		kicked->leaveChannel(c, *comment);
	else
		kicked->leaveChannel(c);
	return ;

noSuchChannel:
	return _send(admin, _architect.ERR_NOSUCHCHANNEL(admin->get_nickname().c_str(), channelName.c_str()));

clientDontHaveThePerm:
	return _send(admin, _architect.ERR_CHANOPRIVSNEEDED(admin->get_nickname().c_str(), channelName.c_str()));

targetDontExist:
	return _send(admin, _architect.ERR_USERNOTINCHANNEL(admin->get_nickname().c_str(), kickedName.c_str(),channelName.c_str()));

adminNotInChannel:
	return _send(admin, _architect.ERR_NOTONCHANNEL(admin->get_nickname().c_str(), channelName.c_str()));
}

#endif
