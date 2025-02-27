#ifndef KICK_H
# define KICK_H

# include <Server.h>
# include <Channel.h>
# include <Client.h>

void	Server::_kickAllChannel(std::vector<str> vecChannel, std::vector<str> vecUser, str *comment, Client *client)
{
	bool	i;
	size_t	k = 0;
	size_t	m = 0;

	if (vecChannel.size() == 1)
		i = 0;
	else if (vecChannel.size() == vecUser.size())
		i = 1;
	else
		goto invalidNumberOfParams;

	for (; k < vecChannel.size() && m < vecUser.size(); k += i, m++)
		_kickChannel(vecChannel[k], client, vecUser[m], comment);
	return ;

invalidNumberOfParams:
		_send(client, _architect.ERR_NEEDMOREPARAMS(client->getTargetName(), "KICK"));
}

IRC_COMMAND_DEF(KICK)
{
	_seeker.feedString(command);
	_seeker.rebuild(R_MIDDLE_PARAM);
	_seeker.findall();
	std::vector<str>	argv = _seeker.get_matches();
	if (argv.size() < 2 || argv.size() > 3)
		return _send(client, _architect.ERR_NEEDMOREPARAMS(client->getTargetName(), "KICK"));


	_seeker.feedString(argv[0]);
	_seeker.rebuild(R_CAPTURE_CHANNEL_NAME);
	_seeker.findall();
	std::vector<str>	vecChannel = _seeker.get_matches();

	_seeker.feedString(argv[1]);
	_seeker.rebuild(R_NICKNAME);
	_seeker.findall();
	std::vector<str>	vecUser = _seeker.get_matches();

	str	*comment = NULL;
	if (argv.size() == 3)
		comment = &argv[2];
	
	_kickAllChannel(vecChannel, vecUser, comment, client);
}

void	Server::_kickChannel(str channelName, Client *admin, str kickedName, str *comment)
{
	Client	*kicked = _getClientByName(kickedName);
	IRC_AUTO		s = _channelMap.find(channelName);
	Channel			*c = NULL;
	int				size;

	if (!kicked)
		goto targetDontExist;
	if (s == _channelMap.end())
		goto noSuchChannel;

	c = s->second;
	try
	{
		if (!c->havePerm(admin->get_fd()))
			goto clientDontHaveThePerm;
	}
	catch (std::exception &e)
	{
		goto adminNotInChannel;
	}
	try
	{
		size = c->removeClient(kicked->get_fd());
		goto succesfullKick;
	}
	catch (std::exception &e)
	{
		goto targetDontExist;
	}

succesfullKick:
	c->_broadcast(_architect.CMD_KICK(admin->get_nickname(), c->get_name().c_str(), kickedName.c_str()));
	if (size == 0)
		_channelMap.erase(s);
	if (comment)
		kicked->leaveChannel(c, *comment);
	else
		kicked->leaveChannel(c);
	return ;

noSuchChannel:
	return _send(admin, _architect.ERR_NOSUCHCHANNEL(admin->getTargetName(), channelName.c_str()));

clientDontHaveThePerm:
	return _send(admin, _architect.ERR_CHANOPRIVSNEEDED(admin->getTargetName(), channelName.c_str()));

targetDontExist:
	return _send(admin, _architect.ERR_USERNOTINCHANNEL(admin->getTargetName(), kickedName.c_str(),channelName.c_str()));

adminNotInChannel:
	return _send(admin, _architect.ERR_NOTONCHANNEL(admin->getTargetName(), channelName.c_str()));
}

#endif
