#pragma once

# include <Server.h>

void	Server::_kickUserFromChannel(const str &channelName, Client &admin, const str &kickedName, const str *comment)
{
	str				reason = comment != NULL ? *comment : "";
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
		if (!c->havePerm(admin.get_fd()))
			goto clientDontHaveThePerm;
	}
	catch (std::exception &e)
	{
		goto adminNotInChannel;
	}
	try
	{
		c->havePerm(kicked->get_fd());
		goto succesfullKick;
	}
	catch (std::exception &e)
	{
		goto targetDontExist;
	}

succesfullKick:
	IRC_LOG("kicked for %s", reason.c_str());
	c->sendMsg(_architect.CMD_KICK(admin.get_nickname(), c->getTargetName(), kickedName.c_str(), reason.c_str()));
	size = c->removeClient(kicked->get_fd());
	if (size == 0)
		_channelMap.erase(s);
	kicked->leaveChannel(c);
	return ;

noSuchChannel:
	return _send(admin, _architect.ERR_NOSUCHCHANNEL(admin.getTargetName(), channelName.c_str()));
clientDontHaveThePerm:
	return _send(admin, _architect.ERR_CHANOPRIVSNEEDED(admin.getTargetName(), channelName.c_str()));
targetDontExist:
	return _send(admin, _architect.ERR_USERNOTINCHANNEL(admin.getTargetName(), kickedName.c_str(),channelName.c_str()));
adminNotInChannel:
	return _send(admin, _architect.ERR_NOTONCHANNEL(admin.getTargetName(), channelName.c_str()));
}

IRC_COMMAND_DEF(KICK)
{
	const std::vector<str>	&param = _parsingParam(command);

	if (param.size() < 2)
		return _send(client, _architect.ERR_NEEDMOREPARAMS(client.getTargetName(), "KICK"));
	if (param.size() > 3)
	{
		IRC_WARN("too many params");
		return ;
	}

	_seeker.feedString(param[0]);
	_seeker.rebuild(R_CAPTURE_CHANNEL_NAME);
	_seeker.findall();
	std::vector<str>	vecChannel = _seeker.get_matches();

	_seeker.feedString(param[1]);
	_seeker.rebuild(R_NICKNAME);
	_seeker.findall();
	std::vector<str>	vecUser = _seeker.get_matches();

	const str	*comment = param.size() == 3 ? &param[2] : NULL;
	
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
		_kickUserFromChannel(vecChannel[k], client, vecUser[m], comment);
	return ;

invalidNumberOfParams:
		_send(client, _architect.ERR_NEEDMOREPARAMS(client.getTargetName(), "KICK"));
}
