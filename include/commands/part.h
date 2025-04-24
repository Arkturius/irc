#pragma once

# include <Server.h>
# include <Channel.h>
#include <exception>
#include <vector>

IRC_COMMAND_DEF(PART)
{
	const int				fd = client.get_fd();
	str						targetName;
	str						partReason;
	const std::vector<str>	&param = _parsingParam(command);
	std::vector<str>		targets;

	_seeker.feedString(command);
	if (param.size() == 0)
		goto needMoreParam;
	if (param.size() > 2)
	{
		IRC_WARN("PART: too many params");
		return ;
	}
	partReason = param.size() == 1 ? "" : param[1];

	_seeker.feedString(param[0]);
	_seeker.rebuild(R_CAPTURE_CHANNEL_NAME);
	_seeker.findall();
	targets = _seeker.get_matches();
	if (targets.size() == 0)
		goto needMoreParam;

	for (IRC_AUTO it = targets.begin(); it != targets.end(); ++it)
	{
		str				targetName = *it;
		IRC_AUTO		s = _channelMap.find(targetName);
		Channel			*target = NULL;

		if (s == _channelMap.end())
			goto noSuchChannel;
		target = s->second;
		try {target->havePerm(fd);}
		catch (std::exception &e) { goto notOnChannel; }

		if (channelIsATable(targetName))
			_clientPartBj(client);
		target->sendMsg(_architect.CMD_PART(client.get_nickname(), target->getTargetName()));
		int size = target->removeClient(fd);
		if (size == 0)
			_channelMap.erase(s);
		client.leaveChannel(target);
	}
	return ;

needMoreParam:
	return _send(client, _architect.ERR_NEEDMOREPARAMS(client.getTargetName(), "PART"));
notOnChannel:
	return _send(client, _architect.ERR_NOTONCHANNEL(client.getTargetName(), targetName.c_str()));
noSuchChannel:
	return _send(client, _architect.ERR_NOSUCHCHANNEL(client.getTargetName(), targetName.c_str()));
}
